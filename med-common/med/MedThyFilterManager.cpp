#include <itkImage.h>
#include <itkImageFileReader.h>
#include "itkImageRegionIterator.h"
#include "itkBinomialBlurImageFilter.h"

#include <itkImageToVTKImageFilter.h>
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkMeanImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"

#include "itkBilateralImageFilter.h"

#include "itkCastImageFilter.h"
#include "itkImageToVTKImageFilter.h"

#include "MedThyFilterManager.h"
#include <functional>

#define FUNC_RIGHT [](ImgPtr ptr, med::MedThy_Param* param)->ImgPtr

#define FILTER_SET_UP(FType)\
    using FilterType = itk::FType<ImageType, ImageType_Float>;\
    auto filter = FilterType::New();\
    filter->SetInput(ptr);\
    if(param->thread_count > 0){\
        filter->SetNumberOfWorkUnits(param->thread_count);\
    }

#define FILTER_FINISH()\
    filter->Update();\
    return med::copyFilterOut(ptr,filter->GetOutput()->GetBufferPointer());

extern ImgPtr do_vtk_smooth(ImgPtr ptr, med::MedThy_Param* param);

namespace med {

void MedThyFilterManager::register_all(){
    m_map[kType_BinomialBlur] = FUNC_RIGHT{
        FILTER_SET_UP(BinomialBlurImageFilter)
        filter->SetRepetitions(param->repeat_count);
        FILTER_FINISH()
    };

    m_map[kType_SmoothingRecursiveGaussian] = FUNC_RIGHT{
        FILTER_SET_UP(SmoothingRecursiveGaussianImageFilter)
        filter->SetSigma(param->sigma);
        FILTER_FINISH()
    };

    m_map[kType_RecursiveGaussian] = FUNC_RIGHT{
        FILTER_SET_UP(RecursiveGaussianImageFilter)
        filter->SetSigma(param->sigma);
        if(param->direction >= 0){
            filter->SetDirection(param->direction);
        }
        if(param->secondOrder){
            filter->SetSecondOrder();
        }
        if(param->directionTolerance >= 0){
            filter->SetDirectionTolerance(param->directionTolerance);
        }
        FILTER_FINISH()
    };

    m_map[kType_Mean] = FUNC_RIGHT{
        FILTER_SET_UP(MeanImageFilter)
        FilterType::InputSizeType radius;
        radius.Fill(param->radius);
        filter->SetRadius(radius);
        FILTER_FINISH()
    };
    m_map[kType_Median] = FUNC_RIGHT{
        FILTER_SET_UP(MedianImageFilter)
        FilterType::InputSizeType radius;
        radius.Fill(param->radius);
        filter->SetRadius(radius);
        FILTER_FINISH()
    };
    m_map[kType_DiscreteGaussian] = FUNC_RIGHT{
        FILTER_SET_UP(DiscreteGaussianImageFilter)
                filter->SetVariance(param->variance);
        FILTER_FINISH()
    };

    m_map[kType_GradientAnisotropicDiffusion] = FUNC_RIGHT{
        FILTER_SET_UP(GradientAnisotropicDiffusionImageFilter)
                filter->SetNumberOfIterations(param->repeat_count);
                if(param->time_step > 0){
                    filter->SetTimeStep(param->time_step);
                }
                filter->SetConductanceParameter(param->conductance);
        FILTER_FINISH()
    };

    m_map[kType_CurvatureAnisotropicDiffusion] = FUNC_RIGHT{
        FILTER_SET_UP(CurvatureAnisotropicDiffusionImageFilter)
                filter->SetNumberOfIterations(param->repeat_count);
                if(param->time_step > 0){
                    filter->SetTimeStep(param->time_step);
                }
                filter->SetConductanceParameter(param->conductance);
        FILTER_FINISH()
    };
    m_map[kType_CurvatureFlow] = FUNC_RIGHT{
        FILTER_SET_UP(CurvatureFlowImageFilter)
        filter->SetNumberOfIterations(param->repeat_count);
        if(param->time_step > 0){
            filter->SetTimeStep(param->time_step);
        }
        FILTER_FINISH()
    };
    //https://examples.itk.org/src/filtering/imagefeature/bilateralfilteranimage/documentation?highlight=blur
    m_map[kType_Bilateral] = FUNC_RIGHT{
            FILTER_SET_UP(BilateralImageFilter)
            filter->SetDomainSigma(param->sigma);
            filter->SetRangeSigma(param->sigma);
            FILTER_FINISH()
    };

    m_map[kType_VTK_Smooth] = do_vtk_smooth;
}

}
