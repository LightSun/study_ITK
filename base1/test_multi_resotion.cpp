#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRecursiveMultiResolutionPyramidImageFilter.h"

using UnsignedCharImageType = itk::Image<unsigned char, 2>;

static void
CreateImage(UnsignedCharImageType::Pointer image);

int
test_multi_resotion()
{
  auto image = UnsignedCharImageType::New();
  CreateImage(image);

  using FloatImageType = itk::Image<float, 2>;

  unsigned int numberOfLevels = 4;

  using RecursiveMultiResolutionPyramidImageFilterType =
    itk::RecursiveMultiResolutionPyramidImageFilter<UnsignedCharImageType, FloatImageType>;
  RecursiveMultiResolutionPyramidImageFilterType::Pointer recursiveMultiResolutionPyramidImageFilter =
    RecursiveMultiResolutionPyramidImageFilterType::New();
  recursiveMultiResolutionPyramidImageFilter->SetInput(image);
  recursiveMultiResolutionPyramidImageFilter->SetNumberOfLevels(numberOfLevels);
  recursiveMultiResolutionPyramidImageFilter->Update();

  // This outputs the levels (0 is the lowest resolution)
  for (unsigned int i = 0; i < numberOfLevels; ++i)
  {
    // Scale so we can write to a PNG
    using RescaleFilterType = itk::RescaleIntensityImageFilter<FloatImageType, UnsignedCharImageType>;
    auto rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(recursiveMultiResolutionPyramidImageFilter->GetOutput(i));
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();

    std::stringstream ss;
    ss << "output_" << i << ".png";
    std::cout << "Writing " << ss.str() << std::endl;
    itk::WriteImage(rescaleFilter->GetOutput(), ss.str());
  }

  return EXIT_SUCCESS;
}


void
CreateImage(UnsignedCharImageType::Pointer image)
{
  // Create a black image with a white region

  UnsignedCharImageType::IndexType start;
  start.Fill(0);

  UnsignedCharImageType::SizeType size;
  size.Fill(200);

  UnsignedCharImageType::RegionType region(start, size);
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(0);

  // Make a square
  for (unsigned int r = 20; r < 80; ++r)
  {
    for (unsigned int c = 30; c < 100; ++c)
    {
      UnsignedCharImageType::IndexType pixelIndex;
      pixelIndex[0] = r;
      pixelIndex[1] = c;

      image->SetPixel(pixelIndex, 255);
    }
  }
}