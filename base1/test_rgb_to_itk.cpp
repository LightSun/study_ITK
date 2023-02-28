#include "vtkSmartPointer.h"
#include "vtkPNGReader.h"
#include "itkVTKImageToImageFilter.h"
#include "itkRGBPixel.h"

void test_rgb_itk()
{
    //TODO
  const char * inputFileName = "";

  constexpr unsigned int Dimension = 2;

  using PixelComponentType = unsigned char;
  using PixelType = itk::RGBPixel<PixelComponentType>;
  using ImageType = itk::Image<PixelType, Dimension>;

  vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
  reader->SetFileName(inputFileName);
  reader->SetDataScalarTypeToUnsignedChar();

  using FilterType = itk::VTKImageToImageFilter<ImageType>;
  auto filter = FilterType::New();
  filter->SetInput(reader->GetOutput());

  try
  {
    reader->Update();
    filter->Update();
  }
  catch (const itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    return;
  }

  ImageType::ConstPointer myitkImage = filter->GetOutput();
  myitkImage->Print(std::cout);

  return;
}
