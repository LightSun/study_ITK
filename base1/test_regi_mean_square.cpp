#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkTranslationTransform.h"

#include "test_common.h"

int
test_registration_mean_square()
{
  String img1 = "";
  String img2 = "";
  using ImageType = itk::Image<double, 2>;

  ImageType::Pointer fixedImage = itk::ReadImage<ImageType>(img1.data());
  ImageType::Pointer movingImage = itk::ReadImage<ImageType>(img2.data());

  using MetricType = itk::MeanSquaresImageToImageMetric<ImageType, ImageType>;
  using InterpolatorType = itk::LinearInterpolateImageFunction<ImageType, double>;
  using TransformType = itk::TranslationTransform<double, 2>;

  auto metric = MetricType::New();
  auto transform = TransformType::New();

  auto interpolator = InterpolatorType::New();
  interpolator->SetInputImage(fixedImage);

  metric->SetFixedImage(fixedImage);
  metric->SetMovingImage(movingImage);
  metric->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion());
  metric->SetTransform(transform);
  metric->SetInterpolator(interpolator);

  TransformType::ParametersType params(transform->GetNumberOfParameters());
  params.Fill(0.0);

  metric->Initialize();
  for (double x = -10.0; x <= 10.0; x += 5.0)
  {
    params(0) = x;
    for (double y = -10.0; y <= 10.0; y += 5.0)
    {
      params(1) = y;
      std::cout << params << ": " << metric->GetValue(params) << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
