#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

//
//曲率各向异性扩散图像滤波器. 会变得光滑
//conductance: 会影响光滑度。越大，越光滑
int
test_curvature_anisotropic(int argc, char * argv[])
{
  if (argc != 6)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0];
    std::cerr << " <InputFileName> <OutputFileName>";
    //default: 5  0.125  3.0
    std::cerr << " <numberOfIterations> <timeStep> <conductance>";
    std::cerr << std::endl;
    return EXIT_FAILURE;
  }

  const char * inputFileName = argv[1];
  const char * outputFileName = argv[2];

  constexpr unsigned int Dimension = 2;

  using InputPixelType = float;
  using InputImageType = itk::Image<InputPixelType, Dimension>;
  using OutputPixelType = unsigned char;
  using OutputImageType = itk::Image<OutputPixelType, Dimension>;

  const int            numberOfIterations = std::stoi(argv[3]);
  const InputPixelType timeStep = std::stod(argv[4]);
  const InputPixelType conductance = std::stod(argv[5]);

  const auto input = itk::ReadImage<InputImageType>(inputFileName);

  using FilterType = itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InputImageType>;
  auto filter = FilterType::New();
  filter->SetInput(input);
  filter->SetNumberOfIterations(numberOfIterations);
  filter->SetTimeStep(timeStep);
  filter->SetNumberOfWorkUnits(4);//thread count
  filter->SetConductanceParameter(conductance);

  using RescaleType = itk::RescaleIntensityImageFilter<InputImageType, OutputImageType>;
  auto rescaler = RescaleType::New();
  rescaler->SetInput(filter->GetOutput());
  rescaler->SetOutputMinimum(itk::NumericTraits<OutputPixelType>::min());
  rescaler->SetOutputMaximum(itk::NumericTraits<OutputPixelType>::max());

  try
  {
    itk::WriteImage(rescaler->GetOutput(), outputFileName);
  }
  catch (const itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
