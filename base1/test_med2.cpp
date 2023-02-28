
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include "itkImageRegionIterator.h"
#include "../nifti_reader.hpp"
#include "../NiftiStateManager.hpp"


h7::NiftiStateManager_U8_3 sNSM;

#define Dimension 3
using ImageType = itk::Image<unsigned char, Dimension>;
using IndexType = itk::Index<Dimension>;

using OutputPixelType = unsigned char;
using InputImageType = itk::Image<OutputPixelType, Dimension>;
using OutputImageType = itk::Image<OutputPixelType, Dimension>;
using FilterOutImageType = itk::ImageSource<ImageType>::OutputImageType;

struct Params{
    int repeat_count;
};

//split
static void tint_nodule_to_thy(ImageType::Pointer p, int nodule_val,
                                 int target);
static void revert(ImageType::Pointer p, int src, int dst);

ImageType::Pointer smooth(ImageType::Pointer p, Params* param);

//1, 将结节颜色设置成甲状腺(并保留结节之前的位置)。
//2，平滑。
//3，将旧结节的位置，颜色还原(注意颜色已经是0的就忽略)。
//可能的问题： 结节和甲状腺相交的地方, 没有平滑
int test_med2(int argc, char * argv[]){
    using namespace std;
    String filename = "/media/heaven7/h7/3d_recreate/1-1_mask.nii";
    if(argc > 1){
        filename = argv[1];
    }
    int repeat_c = 6;
    if(argc > 2){
        repeat_c = stoi(argv[2]);
    }
    String nsm_file;
    if(argc > 3){
        nsm_file = argv[3];
    }
    Params params;
    params.repeat_count = repeat_c;

    if(nsm_file.empty()){
        //
        ImageType::Pointer img1 = h7::read_nii_image_u8(filename.data());
        //1
        tint_nodule_to_thy(img1, 1, 3);
        //2 + 3
        auto out_img = smooth(img1, &params);
        //
        {
            //smooth result
            String path0 = "med2_s_";
            path0 += std::to_string(repeat_c);
            path0 += ".nii";
            itk::WriteImage(out_img, path0);
            sNSM.save("med2_af_smooth.data");
        }

    //    revert(out_img, 3, 1);
    //    String path = "med2_";
    //    path += std::to_string(repeat_c);
    //    path += ".nii";
    //    itk::WriteImage(out_img, path);
    }else{
        sNSM.open(nsm_file);
        ImageType::Pointer img1 = h7::read_nii_image_u8(filename.data());
        revert(img1, 3, 1);
        //
        String path = "med2_";
        path += std::to_string(repeat_c);
        path += ".nii";
        itk::WriteImage(img1, path);
    }
    return 0;
}

void tint_nodule_to_thy(ImageType::Pointer p_nodule, int nodule_val, int target){
    FUNC_START("tint_nodule_to_thy");
    auto size = p_nodule->GetLargestPossibleRegion().GetSize();
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
                int val = p_nodule->GetPixel(index);
                if(val == nodule_val){
                    itk::OffsetValueType offset = p_nodule->ComputeOffset(index);
                    sNSM.addOffset(offset);
                    p_nodule->SetPixel(index, target);
                }
            }
        }
    }
    FUNC_END("tint_nodule_to_thy");
}
static inline bool is_in_old_nodule_pos(ImageType::Pointer p, const IndexType& i){
    itk::OffsetValueType offset = p->ComputeOffset(i);
    return sNSM.containsOffset(offset);
}
ImageType::Pointer smooth(ImageType::Pointer p, Params* param){
    using FilterType = itk::BinomialBlurImageFilter<InputImageType, OutputImageType>;
    FUNC_START("smooth:filter");
    auto filter = FilterType::New();
    filter->SetInput(p);
    filter->SetRepetitions(param->repeat_count);
    filter->SetNumberOfWorkUnits(8);
    filter->Update();
    FUNC_END("smooth:filter");
    FilterOutImageType* fptr = filter->GetOutput();
    auto ptr = fptr->GetBufferPointer();
    if(ptr == NULL){
        fprintf(stderr, "GetBufferPointer = null.\n");
        return nullptr;
    }
    //
    auto size = p->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
    //
    auto out = ImageType::New();
    out->SetRegions(p->GetLargestPossibleRegion());
    out->Allocate();
    memcpy(out->GetBufferPointer(), ptr, cc1 * cc2 * cc3 * sizeof (OutputPixelType));
    return out;
}
static void revert(ImageType::Pointer p, int src, int dst){
    //
    auto size = p->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);

    FUNC_START("revert:loop");
    IndexType index;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val = p->GetPixel(index);
                if(val != 0){
                    if(val == src){
                        if(is_in_old_nodule_pos(p, index)){
                            p->SetPixel(index, dst);
                        }
                    }else{
                        p->SetPixel(index, 0);
                    }
                }
            }
        }
    }
    FUNC_END("revert:loop");
}
