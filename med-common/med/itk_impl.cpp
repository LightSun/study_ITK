#include "itkResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "itkCastImageFilter.h"
#include "itkEllipseSpatialObject.h"
#include "itkImage.h"
#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImageFileWriter.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkResampleImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkTranslationTransform.h"

#include "itkBSplineTransform.h"
#include "itkLBFGSOptimizer.h"

#include "ImageStateManager.hpp"
#include "utils/PerformanceHelper.h"

namespace med {

ImgPtr do_resample(ImgPtr ptr_img, med::ImageState* s){

    if(ptr_img->GetSpacing() == s->spacing
            && ptr_img->GetLargestPossibleRegion().GetSize() == s->size){
        return ptr_img;
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
    return ptr;
}

ImgPtr do_global_registration(ImgPtr fixedImage, ImgPtr movingImage){
    const unsigned int     SpaceDimension = MED_Dimension;
    const unsigned int   ImageDimension = MED_Dimension;
    constexpr unsigned int SplineOrder = 3;
    using CoordinateRepType = double;

     // using TransformType = itk::TranslationTransform<double, MED_Dimension>;
      //  An optimizer is required to explore the parameter space of the transform
      //  in search of optimal values of the metric.
     // using OptimizerType = itk::RegularStepGradientDescentOptimizer;

    using TransformType = itk::BSplineTransform<CoordinateRepType, SpaceDimension, SplineOrder>;
    using OptimizerType = itk::LBFGSOptimizer;

      //  The metric will compare how well the two images match each other. Metric
      //  types are usually parameterized by the image types as it can be seen in
      //  the following type declaration.
      using MetricType = itk::MeanSquaresImageToImageMetric<ImageType, ImageType>;

      //  Finally, the type of the interpolator is declared. The interpolator will
      //  evaluate the intensities of the moving image at non-grid positions.
      using InterpolatorType = itk::LinearInterpolateImageFunction<ImageType, double>;

      //  The registration method type is instantiated using the types of the
      //  fixed and moving images. This class is responsible for interconnecting
      //  all the components that we have described so far.
      using RegistrationType = itk::ImageRegistrationMethod<ImageType, ImageType>;

      // Create components
      auto metric = MetricType::New();
      auto transform = TransformType::New();
      auto optimizer = OptimizerType::New();
      auto interpolator = InterpolatorType::New();
      auto registration = RegistrationType::New();

      // Each component is now connected to the instance of the registration method.
      registration->SetNumberOfWorkUnits(1);
      registration->SetMetric(metric);
      registration->SetOptimizer(optimizer);
      registration->SetTransform(transform);
      registration->SetInterpolator(interpolator);

      // Get the two images
      //auto fixedImage = ImageType::New();
     // auto movingImage = ImageType::New();

      // Set the registration inputs
      registration->SetFixedImage(fixedImage);
      registration->SetMovingImage(movingImage);

//      registration->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion());

//      //  Initialize the transform
//      using ParametersType = RegistrationType::ParametersType;
//      ParametersType initialParameters(transform->GetNumberOfParameters());

//      initialParameters[0] = 0.0; // Initial offset along X
//      initialParameters[1] = 0.0; // Initial offset along Y
//      initialParameters[2] = 0.0;

//      registration->SetInitialTransformParameters(initialParameters);

//      optimizer->SetMaximumStepLength(4.00);
//      optimizer->SetMinimumStepLength(0.01);

//      // Set a stopping criterion
//      optimizer->SetNumberOfIterations(200);

      ImageType::RegionType fixedRegion = fixedImage->GetBufferedRegion();
       registration->SetFixedImageRegion(fixedRegion);

       //  Here we define the parameters of the BSplineDeformableTransform grid.  We
       //  arbitrarily decide to use a grid with $5 \times 5$ nodes within the image.
       //  The reader should note that the BSpline computation requires a
       //  finite support region ( 1 grid node at the lower borders and 2
       //  grid nodes at upper borders). Therefore in this example, we set
       //  the grid size to be $8 \times 8$ and place the grid origin such that
       //  grid node (1,1) coincides with the first pixel in the fixed image.

       TransformType::PhysicalDimensionsType fixedPhysicalDimensions;
       TransformType::MeshSizeType           meshSize;
       for (unsigned int i = 0; i < ImageDimension; ++i)
       {
         fixedPhysicalDimensions[i] =
           fixedImage->GetSpacing()[i] * static_cast<double>(fixedImage->GetLargestPossibleRegion().GetSize()[i] - 1);
       }
       unsigned int numberOfGridNodesInOneDimension = 5;
       meshSize.Fill(numberOfGridNodesInOneDimension - SplineOrder);
       transform->SetTransformDomainOrigin(fixedImage->GetOrigin());
       transform->SetTransformDomainPhysicalDimensions(fixedPhysicalDimensions);
       transform->SetTransformDomainMeshSize(meshSize);
       transform->SetTransformDomainDirection(fixedImage->GetDirection());

       using ParametersType = TransformType::ParametersType;

       const unsigned int numberOfParameters = transform->GetNumberOfParameters();

       ParametersType parameters(numberOfParameters);

       parameters.Fill(0.0);

       transform->SetParameters(parameters);

       //  We now pass the parameters of the current transform as the initial
       //  parameters to be used when the registration process starts.

       registration->SetInitialTransformParameters(transform->GetParameters());

       std::cout << "Intial Parameters = " << std::endl;
       std::cout << transform->GetParameters() << std::endl;

       //  Next we set the parameters of the LBFGS Optimizer.

       optimizer->SetGradientConvergenceTolerance(0.05);
       optimizer->SetLineSearchAccuracy(0.9);
       optimizer->SetDefaultStepLength(.5);
       optimizer->TraceOn();
       optimizer->SetMaximumNumberOfFunctionEvaluations(1000);

       std::cout << std::endl << "Starting Registration" << std::endl;

      // Connect an observer
      // auto observer = CommandIterationUpdate::New();
      // optimizer->AddObserver( itk::IterationEvent(), observer );
      try
      {
        registration->Update();
      }
      catch (const itk::ExceptionObject & err)
      {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        abort();
      }

      //  The result of the registration process is an array of parameters that
      //  defines the spatial transformation in an unique way. This final result is
      //  obtained using the \code{GetLastTransformParameters()} method.

      ParametersType finalParameters = registration->GetLastTransformParameters();
         transform->SetParameters(finalParameters);
      //  In the case of the \doxygen{TranslationTransform}, there is a
      //  straightforward interpretation of the parameters.  Each element of the
      //  array corresponds to a translation along one spatial dimension.

      //const double TranslationAlongX = finalParameters[0];
      //const double TranslationAlongY = finalParameters[1];

      //  The optimizer can be queried for the actual number of iterations
      //  performed to reach convergence.  The \code{GetCurrentIteration()}
      //  method returns this value. A large number of iterations may be an
      //  indication that the maximum step length has been set too small, which
      //  is undesirable since it results in long computational times.

     // const unsigned int numberOfIterations = optimizer->GetCurrentIteration();

      //  The value of the image metric corresponding to the last set of parameters
      //  can be obtained with the \code{GetValue()} method of the optimizer.

      const double bestValue = optimizer->GetValue();

      // Print out results
      //
      std::cout << "Result = " << std::endl;
//      std::cout << " Translation X = " << TranslationAlongX << std::endl;
//      std::cout << " Translation Y = " << TranslationAlongY << std::endl;
     // std::cout << " Iterations    = " << numberOfIterations << std::endl;
      std::cout << " Metric value  = " << bestValue << std::endl;

      //  It is common, as the last step of a registration task, to use the
      //  resulting transform to map the moving image into the fixed image space.
      //  This is easily done with the \doxygen{ResampleImageFilter}. Please
      //  refer to Section~\ref{sec:ResampleImageFilter} for details on the use
      //  of this filter.  First, a ResampleImageFilter type is instantiated
      //  using the image types. It is convenient to use the fixed image type as
      //  the output type since it is likely that the transformed moving image
      //  will be compared with the fixed image.

      using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
      //  A resampling filter is created and the moving image is connected as  its input.

      auto resampler = ResampleFilterType::New();
      resampler->SetInput(movingImage);

      //  The Transform that is produced as output of the Registration method is
      //  also passed as input to the resampling filter. Note the use of the
      //  methods \code{GetOutput()} and \code{Get()}. This combination is needed
      //  here because the registration method acts as a filter whose output is a
      //  transform decorated in the form of a \doxygen{DataObject}. For details in
      //  this construction you may want to read the documentation of the
      //  \doxygen{DataObjectDecorator}.

      resampler->SetTransform(transform);

      //  As described in Section \ref{sec:ResampleImageFilter}, the
      //  ResampleImageFilter requires additional parameters to be specified, in
      //  particular, the spacing, origin and size of the output image. The default
      //  pixel value is also set to a distinct gray level in order to highlight
      //  the regions that are mapped outside of the moving image.

      resampler->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
      resampler->SetOutputOrigin(fixedImage->GetOrigin());
      resampler->SetOutputSpacing(fixedImage->GetSpacing());
      resampler->SetOutputDirection(fixedImage->GetDirection());
      resampler->SetDefaultPixelValue(0);

      //  The output of the filter is passed to a writer that will store the
      //  image in a file. An \doxygen{CastImageFilter} is used to convert the
      //  pixel type of the resampled image to the final type used by the
      //  writer. The cast and writer filters are instantiated below.

//      using CastFilterType = itk::CastImageFilter<ImageType, ImageType>;

//      auto caster = CastFilterType::New();
//      caster->SetInput(resampler->GetOutput());

      return resampler->GetOutput();
}

}

