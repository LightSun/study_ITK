#ifndef ITKFLOW_PRI_H
#define ITKFLOW_PRI_H

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"

#include "MedFlow_ctx.h"

#include "common/logger.h"
#include "common/common.h"

#define LOGD h7_logd
#define LOGI h7_logi
#define LOGW h7_logw
#define LOGE h7_loge

#define MED_Dimension 3
using IndexType = itk::Index<MED_Dimension>;
using ImageType = itk::Image<float, MED_Dimension>;
using ImageType_Float = itk::Image<float, MED_Dimension>;
using ImageType_I8 = itk::Image<unsigned char, MED_Dimension>;

typedef ImageType::Pointer ImgPtr;
typedef ImgPtr (*Func_Filter)(ImgPtr, med::MedThy_Param*);

namespace med{

struct _MedThyContext{
    ImageType::Pointer ptr_raw;
    ImageType::Pointer ptr_thy;
    ImageType::Pointer ptr_nodule_good;
    ImageType::Pointer ptr_nodule_bad;
    ImageType_I8::Pointer ptr_merge;
};

struct ImageState{
    ImageType::SizeType size;
    ImageType::SpacingType spacing;
    ImageType::PointType origin;
    ImageType::DirectionType direction;

    void print(){
        std::string out;
        HFMT_BUF_128({
                     out += buf;
                     }, "size = %d, %d, %d\n",
                     (int)size[0], (int)size[1], (int)size[2]);
        HFMT_BUF_128({
                     out += buf;
                     }, "spacing = %d, %d, %d\n",
                     (int)spacing[0], (int)spacing[1], (int)spacing[2]);
        LOGD("ImageState >> %s", out.data());
    }
};

static inline ImageType::Pointer copyFilterOut(ImageType::Pointer p,
                                                 const void* ptr_buf){
    auto out = ImageType::New();
    out->SetSpacing(p->GetSpacing());
    out->SetRegions(p->GetLargestPossibleRegion());
    out->Allocate();
    auto size = p->GetLargestPossibleRegion().GetSize();

    memcpy(out->GetBufferPointer(), ptr_buf,
           size.GetElement(0) * size.GetElement(1) * size.GetElement(2)
            * sizeof (ImageType::PixelType)
           );
    return out;
}
extern ImgPtr do_resample(ImgPtr ptr_img, med::ImageState* s);
extern ImgPtr do_global_registration(ImgPtr fixedImage, ImgPtr movingImage);

}



#endif // ITKFLOW_PRI_H
