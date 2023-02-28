#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "itkRGBPixel.h"

#include <string>
typedef std::string String;

void test_rgb_to_vtk()
{
  String dir = "/home/heaven7/heaven7/libs/ITK/sample/InsightData-5.2.1/"
               "InsightToolkit-5.2.1/.ExternalData/MD5/";
  String inputFileName = dir + "00bd97f6008825bab8e117837777e9fe";

  constexpr unsigned int Dimension = 2;

  using PixelComponentType = unsigned char;
  using PixelType = itk::RGBPixel<PixelComponentType>;
  using ImageType = itk::Image<PixelType, Dimension>;

  const auto input = itk::ReadImage<ImageType>(inputFileName.data());

  using FilterType = itk::ImageToVTKImageFilter<ImageType>;
  auto filter = FilterType::New();
  filter->SetInput(input);

  try
  {
    filter->Update();
  }
  catch (const itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    return;
  }

  vtkImageData * myvtkImageData = filter->GetOutput();
  myvtkImageData->Print(std::cout);
}
