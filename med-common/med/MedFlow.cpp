
#include "MedFlow.hpp"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinomialBlurImageFilter.h"

#include <itkNiftiImageIO.h>
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "MedThyFilterManager.h"
#include "ImageStateManager.hpp"
#include "common/common.h"


using ReaderType = itk::ImageFileReader<ImageType>;

using InputImageType = ImageType;
using OutputImageType = ImageType;

#define filterM med::MedThyFilterManager::get()

static inline ImageType::Pointer do_filter(med::MedThyContext* ctx,int flowType,
                                 int filterType, bool onlyTwoType,
                                 med::MedThy_Param* lp);

template <typename TImage>
void
DeepCopy(typename TImage::Pointer input, typename TImage::Pointer output)
{
  output->SetSpacing(input->GetSpacing());
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImage>      outputIterator(output, output->GetLargestPossibleRegion());

  while (!inputIterator.IsAtEnd())
  {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
  }
}

static inline ImgPtr getImagePtrByType(med::MedThyContext* ctx, int flowType){
    ImageType::Pointer ptr_in;
    switch (flowType) {
    case med::kITKFLOW_THY:{
        ptr_in = ctx->ptr_thy;
    }break;

    case med::kITKFLOW_NODULE_GOOD:{
        ptr_in = ctx->ptr_nodule_good;
    }break;

    case med::kITKFLOW_NODULE_BAD:{
        ptr_in = ctx->ptr_nodule_bad;
    }break;

    default:
        LOGE("getImagePtrByType: wrong type = %d\n", flowType);
        abort();
    }
    return  ptr_in;
}
static inline void setImagePtrByType(med::MedThyContext* ctx, int flowType, ImgPtr dst){
    switch (flowType) {
    case med::kITKFLOW_THY:{
        ctx->ptr_thy = dst;
    }break;

    case med::kITKFLOW_NODULE_GOOD:{
        ctx->ptr_nodule_good = dst;
    }break;

    case med::kITKFLOW_NODULE_BAD:{
        ctx->ptr_nodule_bad = dst;
    }break;

    default:
        LOGE("setImagePtrByType: wrong type = %d\n", flowType);
    }
}

namespace med {
MedThyFlow::MedThyFlow(const InputParams& lfp):m_params(lfp){
    m_ctx = new _MedThyContext();
    m_stateM = new ImageStateManager();
}
MedThyFlow::~MedThyFlow(){
    if(m_ctx){
        delete m_ctx;
        m_ctx = nullptr;
    }
    if(m_stateM){
        delete m_stateM;
        m_stateM = nullptr;
    }
}
void MedThyFlow::read_nifti(CString path){
    LOGD("read_nifti >> %s\n", path.data());
    ReaderType::Pointer reader = ReaderType::New();
    itk::NiftiImageIO::Pointer niftiIO = itk::NiftiImageIO::New();
    reader->SetImageIO(niftiIO);
    reader->SetFileName(path);
    reader->Update();
    m_ctx->ptr_raw = reader->GetOutput();
}

void MedThyFlow::split_data(){
    LOGD("split_data >>\n");
    m_ctx->ptr_thy = ImageType::New();
    DeepCopy<ImageType>(m_ctx->ptr_raw, m_ctx->ptr_thy);
    //
    ImageType::Pointer ptr = m_ctx->ptr_thy;

    ImageType::Pointer ptr_good = ImageType::New();
    ptr_good->SetSpacing(ptr->GetSpacing());
    ptr_good->SetRegions(m_ctx->ptr_thy->GetLargestPossibleRegion());
    ptr_good->Allocate();
    ptr_good->FillBuffer(kITKFLOW_NONE);

    ImageType::Pointer ptr_bad = ImageType::New();
    ptr_bad->SetSpacing(ptr->GetSpacing());
    ptr_bad->SetRegions(m_ctx->ptr_thy->GetLargestPossibleRegion());
    ptr_bad->Allocate();
    ptr_bad->FillBuffer(kITKFLOW_NONE);
    //
    auto size = m_ctx->ptr_thy->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
    IndexType index;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val = m_ctx->ptr_thy->GetPixel(index);
                switch (val) {
                case kITKFLOW_THY:
                {
                    //do nothing
                    //ptr_good->SetPixel(index, kITKFLOW_NONE);
                    //ptr_bad->SetPixel(index, kITKFLOW_NONE);
                }break;

                case kITKFLOW_NODULE_GOOD:
                {
                    ptr_good->SetPixel(index, kITKFLOW_NODULE_GOOD);
                    ptr->SetPixel(index, kITKFLOW_NONE);
                }break;

                case kITKFLOW_NODULE_BAD:
                {
                    ptr_bad->SetPixel(index, kITKFLOW_NODULE_BAD);
                    ptr->SetPixel(index, kITKFLOW_NONE);
                }break;

                default:
                case kITKFLOW_NONE:
                {
                    //do nothing
                }break;
                }
            }
        }
    }
    //
    m_ctx->ptr_nodule_good = ptr_good;
    m_ctx->ptr_nodule_bad = ptr_bad;
}

void MedThyFlow::resamples(int type, float* spacing, int space_count){
    LOGD("resamples >> type = %d\n", type);
    (void)space_count;
    auto ptr_img = getImagePtrByType(m_ctx, type);
    //
    ImageType::SpacingType inputSpacing = ptr_img->GetSpacing();
    ImageType::SizeType    inputSize = ptr_img->GetLargestPossibleRegion().GetSize();

    ImageType::SizeType    outputSize;
   // outputSize[0] = inputSize[0] * inputSpacing[0] / spacing[0];
//    outputSize[0] = inputSize[0] * size_ratio[0];
//    outputSize[1] = inputSize[1] * size_ratio[1];
//    outputSize[2] = inputSize[2] * size_ratio[2];
    outputSize[0] = inputSize[0] / spacing[0];
    outputSize[1] = inputSize[1] / spacing[1];
    outputSize[2] = inputSize[2] / spacing[2];

    ImageType::SpacingType outSpaces;
    outSpaces[0] = inputSpacing[0] * spacing[0];
    outSpaces[1] = inputSpacing[1] * spacing[1];
    outSpaces[2] = inputSpacing[2] * spacing[2];

    ImageState s;
    s.size = std::move(outputSize);
    s.spacing = std::move(outSpaces);
    s.origin = ptr_img->GetOrigin();
    s.direction = ptr_img->GetDirection();

    apply_resample(m_ctx, type, &s);
}

void MedThyFlow::smooth_thy(int type,bool onlyTwoTypeValue){
    LOGD("smooth_thy >>\n");
    m_ctx->ptr_thy = do_filter(m_ctx, kITKFLOW_THY, type,
                               onlyTwoTypeValue, &m_params.thy);
}

void MedThyFlow::smooth_nodules(int type,bool onlyTwoTypeValue){
    if(m_ctx->ptr_nodule_good){
        LOGD("smooth_nodules >> good.\n");
        m_ctx->ptr_nodule_good = do_filter(m_ctx, kITKFLOW_NODULE_GOOD, type,
                                           onlyTwoTypeValue, &m_params.nodule_good);
    }
    if(m_ctx->ptr_nodule_bad){
        LOGD("smooth_nodules >> bad.\n");
        m_ctx->ptr_nodule_bad = do_filter(m_ctx, kITKFLOW_NODULE_BAD, type,
                                          onlyTwoTypeValue, &m_params.nodule_bad);
    }
}

void MedThyFlow::merge(){

    LOGD("merge >>.\n");
    ImageType::Pointer p_good = m_ctx->ptr_nodule_good;
    ImageType::Pointer p_bad = m_ctx->ptr_nodule_bad;
    ImageType::Pointer p_thy = m_ctx->ptr_thy;

    auto ptr = ImageType_I8::New();
    ptr->SetSpacing(p_thy->GetSpacing());
    ptr->SetRegions(p_thy->GetLargestPossibleRegion());
    ptr->Allocate();
    ptr->FillBuffer(kITKFLOW_NONE);
    //
    auto size = p_thy->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);

    IndexType index;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val_good = p_good->GetPixel(index);
                int dst;
                if(val_good != kITKFLOW_NONE){
                    dst = val_good;
                }else{
                    int val_bad = p_bad->GetPixel(index);
                    if(val_bad != kITKFLOW_NONE){
                        dst = val_bad;
                    }else{
                        int val_thy = p_thy->GetPixel(index);
                        dst = val_thy == kITKFLOW_NONE ? kITKFLOW_NONE : kITKFLOW_THY;
                    }
                }
                ptr->SetPixel(index, dst);
            }
        }
    }
    m_ctx->ptr_merge = ptr;
}

#define _Println(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__);

void MedThyFlow::save_thy(CString filename){
    LOGD("save_thy >> %s\n", filename.data());
    if(m_ctx->ptr_thy){
        itk::WriteImage(m_ctx->ptr_thy, filename);
    }else{
        _Println("dump_thy: but thy data not exist!\n");
    }
}
void MedThyFlow::save_nodules(CString fn_good, CString fn_bad){
    LOGD("save_nodules >> good-> %s\n", fn_good.data());
    LOGD("save_nodules >> bad-> %s\n", fn_bad.data());
    if(m_ctx->ptr_nodule_good){
        itk::WriteImage(m_ctx->ptr_nodule_good, fn_good);
    }else{
        _Println("save_nodules: but ptr_nodule_good data not exist!\n");
    }
    if(m_ctx->ptr_nodule_bad){
        itk::WriteImage(m_ctx->ptr_nodule_bad, fn_bad);
    }else{
        _Println("save_nodules: but ptr_nodule_bad data not exist!\n");
    }
}
void MedThyFlow::save_merge_result(CString filename){
    LOGD("save_merge_result >> %s\n", filename.data());
    if(m_ctx->ptr_merge){
        itk::WriteImage(m_ctx->ptr_merge, filename);
    }else{
        _Println("dump_nodules: but ptr_merge data not exist!\n");
    }
}

void MedThyFlow::load_nifti(int type, CString filename){
    LOGD("load_nifti: %s\n", filename.data());
    ReaderType::Pointer reader = ReaderType::New();
    itk::NiftiImageIO::Pointer niftiIO = itk::NiftiImageIO::New();
    reader->SetImageIO(niftiIO);
    reader->SetFileName(filename);
    reader->Update();
    //
    setImagePtrByType(m_ctx, type, reader->GetOutput());
}

void MedThyFlow::save_imageState(int type){
    LOGD("save_imageState: %d\n", type);
    if(m_stateM){
        auto ptr = getImagePtrByType(m_ctx, type);
        m_stateM->saveState(type, ptr);
    }
}
void MedThyFlow::restore_imageState(int type){
    LOGD("restore_imageState: %d\n", type);
    ImageState s;
    MED_ASSERT(m_stateM->removeState(type, &s));
    apply_resample(m_ctx, type, &s);
}

void MedThyFlow::apply_resample(MedThyContext* ctx, int type, void* ptr_s){
    ImageState* s = (med::ImageState*)ptr_s;
    s->print();
    auto ptr_img = getImagePtrByType(ctx, type);
    if(ptr_img->GetSpacing() == s->spacing
            && ptr_img->GetLargestPossibleRegion().GetSize() == s->size){
        return;
    }
    //
    h7::PerfHelper ph;
    ph.begin();
    typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleImageFilterType;
    ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New();
    resampleFilter->SetInput(ptr_img);

    typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double> InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    resampleFilter->SetInterpolator(interpolator);
    resampleFilter->SetDefaultPixelValue(0);//0 is default
    resampleFilter->SetOutputSpacing(s->spacing);
    //resampleFilter->SetSize(ptr_img->GetLargestPossibleRegion().GetSize());
    resampleFilter->SetSize(s->size);
    resampleFilter->SetOutputOrigin(s->origin);
    resampleFilter->SetOutputDirection(s->direction);
    resampleFilter->Update();
    auto cost_time = ph.formatTime(ph.end());
    LOGD("apply_resample >> cost %s\n", cost_time.data());

    auto ptr = resampleFilter->GetOutput();
    setImagePtrByType(ctx, type, ptr);
}

}

//------------------------
static ImageType::Pointer do_filter(med::MedThyContext* ctx,int flowType, int filterType,
                                    bool onlyTwoType, med::MedThy_Param* lp){
    ImageType::Pointer ptr_in = getImagePtrByType(ctx, flowType);

    auto ptr = filterM->apply_filter(filterType, ptr_in, lp);
    if(!ptr){
        fprintf(stderr, "can't find filter type(%d).\n", filterType);
        abort();
    }
    if(onlyTwoType){
        auto size = ptr->GetLargestPossibleRegion().GetSize();
        int cc1 = size.GetElement(0);
        int cc2 = size.GetElement(1);
        int cc3 = size.GetElement(2);

        IndexType index;
        for(int i = 0 ; i < cc1 ; ++i){
            index[0] = i;
            for(int j = 0 ; j < cc2 ; ++j){
                index[1] = j;
                for(int k = 0 ; k < cc3 ; ++k){
                    index[2] = k;
                    int val = ptr->GetPixel(index);
                    ptr->SetPixel(index, val != med::kITKFLOW_NONE
                            ? flowType : med::kITKFLOW_NONE);
                }
            }
        }
    }
    return ptr;
}



