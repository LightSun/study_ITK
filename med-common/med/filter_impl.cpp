#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkVectorCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkVectorToRGBImageAdaptor.h"
#include "itkRGBToVectorImageAdaptor.h"
#include "itkCastImageFilter.h"

#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkVectorMagnitudeImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkComposeImageFilter.h"

#include "itkCurvatureAnisotropicDiffusionImageFilter.h"

#include "itkCastImageFilter.h"

#include "itksys/SystemTools.hxx"
#include "MedFlow_pri.h"

using VecFloatImageType = itk::Image<itk::Vector<float, 3>, MED_Dimension>;

static inline ImageType::Pointer copyFilterOut(ImageType::Pointer p,
                                                 const void* ptr_buf){
    auto out = ImageType::New();
    out->SetRegions(p->GetLargestPossibleRegion());
    out->Allocate();
    memcpy(out->GetBufferPointer(), ptr_buf, p->GetNumberOfComponentsPerPixel()
           * p->GetLargestPossibleRegion().GetNumberOfPixels());
    return out;
}

#define FUNC_RIGHT [](ImgPtr ptr, med::MedThy_Param* param)->ImgPtr


#define FILTER_SET_UP(FType)\
    using FilterType = itk::FType<ImageType, ImageType_F>;\
    auto filter = FilterType::New();\
    filter->SetInput(ptr);\
    if(param->thread_count > 0){\
        filter->SetNumberOfWorkUnits(param->thread_count);\
    }

#define FILTER_FINISH()\
    filter->Update();\
    return copyFilterOut(ptr,filter->GetOutput()->GetBufferPointer());

ImageType::Pointer doFilter_vec_cur_ani_diffusion(ImageType::Pointer ptr,
                                med::MedThy_Param* param){
    //using VecImageType = itk::VectorImage<float>;

    using VecImageType = itk::Image<itk::Vector<float, MED_Dimension>, MED_Dimension>;

    using VecPixType = VecImageType::PixelType;
    auto ptr_vec = VecImageType::New();
    ptr_vec->SetRegions(ptr->GetLargestPossibleRegion());
    ptr_vec->Allocate();
    //
    auto size = ptr->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
    typedef itk::RGBPixel<unsigned char>    PixelType;
    IndexType index;
    using VL_Vec_Float = itk::Vector<float, MED_Dimension>;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val = ptr->GetPixel(index);
                VL_Vec_Float var;
                var[0] = (val >> 16) & 0xff;
                var[1] = (val >> 8) & 0xff;
                var[2] = val & 0xff;
                ptr_vec->SetPixel(index, var);
            }
        }
    }

    using FilterType = itk::CurvatureAnisotropicDiffusionImageFilter
                <VecImageType, ImageType_F>;
    auto filter = FilterType::New();
    filter->SetInput(ptr_vec);
    if(param->thread_count > 0){
        filter->SetNumberOfWorkUnits(param->thread_count);
    }
    //FILTER_SET_UP(VectorCurvatureAnisotropicDiffusionImageFilter)
    filter->SetNumberOfIterations(param->repeat_count);
    filter->SetTimeStep(ptr->GetTimeStamp());
    filter->SetConductanceParameter(param->conductance);
    FILTER_FINISH()
}
