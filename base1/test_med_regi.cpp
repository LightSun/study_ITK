

#include "itkImageRegistrationMethod.h"
#include "itkTranslationTransform.h"
#include "itkMutualInformationImageToImageMetric.h"
#include "itkGradientDescentOptimizer.h"
#include "itkNormalizeImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCheckerBoardImageFilter.h"
#include "itkEllipseSpatialObject.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include "itkImageRegionIterator.h"
#include "../nifti_reader.hpp"

constexpr unsigned int Dimension = 3;
using PixelType = unsigned char;
using ImageType = itk::Image<PixelType, Dimension>;
using IndexType = itk::Index<Dimension>;

using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;

#define fixedImage p_thy
#define movingImage p_nodule

//
static ResampleFilterType::Pointer merge1(ImageType::Pointer p_nodule, ImageType::Pointer p_thy);

static ImageType::Pointer merge2(ImageType::Pointer p_nodule, ImageType::Pointer p_thy);


int test_med_regi(int argc, char* argv[]){
    String path_img1 = "img1_1.nii";
    String path_img2 = "img2_1.nii";
    //img1 is the nodule
    ImageType::Pointer img1 = h7::read_nii_image_u8(path_img1.data());
    ImageType::Pointer img2 = h7::read_nii_image_u8(path_img2.data());
    auto out = merge2(img1, img2);
    itk::WriteImage(out, "med_regi_2.nii");

//    auto out = merge1(img1, img2);
//    itk::WriteImage(out->GetOutput(), "med_regi_1.nii");
    return 0;
}

static ImageType::Pointer merge2(ImageType::Pointer p_nodule, ImageType::Pointer p_thy){
    auto ptr = ImageType::New();
    ptr->SetRegions(p_thy->GetLargestPossibleRegion());
    ptr->Allocate();
    ptr->FillBuffer(0);

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
                int val_nod = p_nodule->GetPixel(index);
                int val_thy = p_thy->GetPixel(index);
                int dst;
                if(val_nod == 0){
                    //3 is thy good
                    dst = val_thy == 0 ? 0 : 3;
                }else{
                    //nodule is important
                    dst = val_nod;
                }
                ptr->SetPixel(index, dst);
            }
        }
    }
    return ptr;
}

static ResampleFilterType::Pointer merge1(ImageType::Pointer p_nodule, ImageType::Pointer p_thy){
//      p_out->SetRegions(p_nodule->GetLargestPossibleRegion());
//      p_out->Allocate();
//      p_out->FillBuffer(0);
      // We use floats internally
      using InternalImageType = itk::Image<float, Dimension>;

      // Normalize the images
      using NormalizeFilterType = itk::NormalizeImageFilter<ImageType, InternalImageType>;

      auto fixedNormalizer = NormalizeFilterType::New();
      auto movingNormalizer = NormalizeFilterType::New();

      fixedNormalizer->SetInput(fixedImage);
      movingNormalizer->SetInput(movingImage);

      // Smooth the normalized images
      using GaussianFilterType = itk::DiscreteGaussianImageFilter<InternalImageType, InternalImageType>;

      auto fixedSmoother = GaussianFilterType::New();
      auto movingSmoother = GaussianFilterType::New();

      fixedSmoother->SetVariance(2.0);
      movingSmoother->SetVariance(2.0);

      fixedSmoother->SetInput(fixedNormalizer->GetOutput());
      movingSmoother->SetInput(movingNormalizer->GetOutput());

      using TransformType = itk::TranslationTransform<double, Dimension>;
      using OptimizerType = itk::GradientDescentOptimizer;
      using InterpolatorType = itk::LinearInterpolateImageFunction<InternalImageType, double>;
      using RegistrationType = itk::ImageRegistrationMethod<InternalImageType, InternalImageType>;
      using MetricType = itk::MutualInformationImageToImageMetric<InternalImageType, InternalImageType>;

      auto transform = TransformType::New();
      auto optimizer = OptimizerType::New();
      auto interpolator = InterpolatorType::New();
      auto registration = RegistrationType::New();

      registration->SetOptimizer(optimizer);
      registration->SetTransform(transform);
      registration->SetInterpolator(interpolator);

      auto metric = MetricType::New();
      registration->SetMetric(metric);

      //  The metric requires a number of parameters to be selected, including
      //  the standard deviation of the Gaussian kernel for the fixed image
      //  density estimate, the standard deviation of the kernel for the moving
      //  image density and the number of samples use to compute the densities
      //  and entropy values. Details on the concepts behind the computation of
      //  the metric can be found in Section
      //  \ref{sec:MutualInformationMetric}.  Experience has
      //  shown that a kernel standard deviation of $0.4$ works well for images
      //  which have been normalized to a mean of zero and unit variance.  We
      //  will follow this empirical rule in this example.

      metric->SetFixedImageStandardDeviation(0.4);
      metric->SetMovingImageStandardDeviation(0.4);

      registration->SetFixedImage(fixedSmoother->GetOutput());
      registration->SetMovingImage(movingSmoother->GetOutput());

      fixedNormalizer->Update();
      ImageType::RegionType fixedImageRegion = fixedNormalizer->GetOutput()->GetBufferedRegion();
      registration->SetFixedImageRegion(fixedImageRegion);

      using ParametersType = RegistrationType::ParametersType;
      ParametersType initialParameters(transform->GetNumberOfParameters());

      initialParameters[0] = 0.0; // Initial offset along X
      initialParameters[1] = 0.0; // Initial offset along Y
      initialParameters[2] = 0.0;

      registration->SetInitialTransformParameters(initialParameters);

      //  Software Guide : BeginLatex
      //
      //  We should now define the number of spatial samples to be considered in
      //  the metric computation. Note that we were forced to postpone this setting
      //  until we had done the preprocessing of the images because the number of
      //  samples is usually defined as a fraction of the total number of pixels in
      //  the fixed image.
      //
      //  The number of spatial samples can usually be as low as $1\%$ of the total
      //  number of pixels in the fixed image. Increasing the number of samples
      //  improves the smoothness of the metric from one iteration to another and
      //  therefore helps when this metric is used in conjunction with optimizers
      //  that rely of the continuity of the metric values. The trade-off, of
      //  course, is that a larger number of samples result in longer computation
      //  times per every evaluation of the metric.
      //
      //  It has been demonstrated empirically that the number of samples is not a
      //  critical parameter for the registration process. When you start fine
      //  tuning your own registration process, you should start using high values
      //  of number of samples, for example in the range of $20\%$ to $50\%$ of the
      //  number of pixels in the fixed image. Once you have succeeded to register
      //  your images you can then reduce the number of samples progressively until
      //  you find a good compromise on the time it takes to compute one evaluation
      //  of the Metric. Note that it is not useful to have very fast evaluations
      //  of the Metric if the noise in their values results in more iterations
      //  being required by the optimizer to converge. You must then study the
      //  behavior of the metric values as the iterations progress.

      const unsigned int numberOfPixels = fixedImageRegion.GetNumberOfPixels();

      const auto numberOfSamples = static_cast<unsigned int>(numberOfPixels * 0.01);

      metric->SetNumberOfSpatialSamples(numberOfSamples);

      // For consistent results when regression testing.
      metric->ReinitializeSeed(121212);

      //  Since larger values of mutual information indicate better matches than
      //  smaller values, we need to maximize the cost function in this example.
      //  By default the GradientDescentOptimizer class is set to minimize the
      //  value of the cost-function. It is therefore necessary to modify its
      //  default behavior by invoking the \code{MaximizeOn()} method.
      //  Additionally, we need to define the optimizer's step size using the
      //  \code{SetLearningRate()} method.

      optimizer->SetLearningRate(15.0);
      optimizer->SetNumberOfIterations(200);
      optimizer->MaximizeOn(); // We want to maximize mutual information (the default of the optimizer is to minimize)

      // Note that large values of the learning rate will make the optimizer
      // unstable. Small values, on the other hand, may result in the optimizer
      // needing too many iterations in order to walk to the extrema of the cost
      // function. The easy way of fine tuning this parameter is to start with
      // small values, probably in the range of $\{5.0,10.0\}$. Once the other
      // registration parameters have been tuned for producing convergence, you
      // may want to revisit the learning rate and start increasing its value until
      // you observe that the optimization becomes unstable.  The ideal value for
      // this parameter is the one that results in a minimum number of iterations
      // while still keeping a stable path on the parametric space of the
      // optimization. Keep in mind that this parameter is a multiplicative factor
      // applied on the gradient of the Metric. Therefore, its effect on the
      // optimizer step length is proportional to the Metric values themselves.
      // Metrics with large values will require you to use smaller values for the
      // learning rate in order to maintain a similar optimizer behavior.

      try
      {
        registration->Update();
        std::cout << "Optimizer stop condition: " << registration->GetOptimizer()->GetStopConditionDescription()
                  << std::endl;
      }
      catch (const itk::ExceptionObject & err)
      {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        return nullptr;
      }

      ParametersType finalParameters = registration->GetLastTransformParameters();

      double TranslationAlongX = finalParameters[0];
      double TranslationAlongY = finalParameters[1];
      double TranslationAlongZ = finalParameters[2];

      unsigned int numberOfIterations = optimizer->GetCurrentIteration();

      double bestValue = optimizer->GetValue();


      // Print out results
      std::cout << std::endl;
      std::cout << "Result = " << std::endl;
      std::cout << " Translation X = " << TranslationAlongX << std::endl;
      std::cout << " Translation Y = " << TranslationAlongY << std::endl;
      std::cout << " Translation Z = " << TranslationAlongZ << std::endl;
      std::cout << " Iterations    = " << numberOfIterations << std::endl;
      std::cout << " Metric value  = " << bestValue << std::endl;
      std::cout << " Numb. Samples = " << numberOfSamples << std::endl;

     // using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;

      auto finalTransform = TransformType::New();

      finalTransform->SetParameters(finalParameters);
      finalTransform->SetFixedParameters(transform->GetFixedParameters());

      auto resample = ResampleFilterType::New();

      resample->SetTransform(finalTransform);
      resample->SetInput(movingImage);

      resample->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
      resample->SetOutputOrigin(fixedImage->GetOrigin());
      resample->SetOutputSpacing(fixedImage->GetSpacing());
      resample->SetOutputDirection(fixedImage->GetDirection());
      resample->SetDefaultPixelValue(0);

     // itk::WriteImage(resample->GetOutput(), out.data());

      return resample;
}

