#ifndef ITKFLOW_PRI_H
#define ITKFLOW_PRI_H

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"

#include "MedFlow_ctx.h"

#include "common/logger.h"

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
}

#endif // ITKFLOW_PRI_H
