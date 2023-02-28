#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"

#define ENABLE_QUICKVIEW 1

#ifdef ENABLE_QUICKVIEW
#  include "QuickView.h"
#endif

//高阶导可用于提取轮廓
int
test_high_derivative(int argc, char * argv[])
{
  // Verify command line arguments
  if (argc < 2)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "inputImageFile" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse command line arguments
  std::string inputFileName = argv[1];

  // Setup types
  using FloatImageType = itk::Image<float, 2>;
  using UnsignedCharImageType = itk::Image<unsigned char, 2>;

  using filterType = itk::RecursiveGaussianImageFilter<UnsignedCharImageType, FloatImageType>;

  const auto input = itk::ReadImage<UnsignedCharImageType>(inputFileName);

  // Create and setup a gaussian filter
  auto gaussianFilter = filterType::New();
  gaussianFilter->SetInput(input);
  gaussianFilter->SetDirection(0); // "x" axis
  gaussianFilter->SetSecondOrder();

#ifdef ENABLE_QUICKVIEW
  QuickView viewer;
  viewer.AddImage<UnsignedCharImageType>(input);
  viewer.AddImage<FloatImageType>(gaussianFilter->GetOutput());
  viewer.Visualize();
#endif

  return EXIT_SUCCESS;
}
