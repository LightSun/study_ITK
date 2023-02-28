#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkVectorCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkVectorToRGBImageAdaptor.h"
#include "itkRGBToVectorImageAdaptor.h"
#include "itkCastImageFilter.h"

#include "itksys/SystemTools.hxx"
#include <sstream>

#include "test_common.h"

#define ENABLE_QUICKVIEW 1
#ifdef ENABLE_QUICKVIEW
#  include "QuickView.h"
#endif

//在保留边缘的同时平滑图像
int
test_curvature_preserve_edge(int argc, char * argv[])
{
  // Verify arguments
//  if (argc < 2)
//  {
//    std::cerr << "Usage: " << std::endl;
//    std::cerr << argv[0];
//    std::cerr << " InputFileName";
//    std::cerr << " [NumberOfIterations] ";
//    std::cerr << " [Conductance]" << std::endl;
//    return EXIT_FAILURE;
//  }

  // 0) Parse arguments
  std::string inputFileName = "1.png";
  if(argc > 1){
      inputFileName = argv[1];
  }

  using FloatImageType = itk::Image<itk::Vector<float, 3>, 2>;
  using RGBImageType = itk::Image<itk::RGBPixel<float>, 2>;

  // 1) Read the RGB image
  const auto input = itk::ReadImage<RGBImageType>(inputFileName);

  // 2) Cast to Vector image for processing
  using AdaptorInputType = itk::RGBToVectorImageAdaptor<RGBImageType>;
  auto adaptInput = AdaptorInputType::New();
  adaptInput->SetImage(input);
  using CastInputType = itk::CastImageFilter<AdaptorInputType, FloatImageType>;
  auto castInput = CastInputType::New();
  castInput->SetInput(adaptInput);

  // 3) Smooth the image
  using VectorCurvatureAnisotropicDiffusionImageFilterType =
    itk::VectorCurvatureAnisotropicDiffusionImageFilter<FloatImageType, FloatImageType>;
  VectorCurvatureAnisotropicDiffusionImageFilterType::Pointer filter =
    VectorCurvatureAnisotropicDiffusionImageFilterType::New();
  filter->SetInput(castInput->GetOutput());
  filter->SetTimeStep(0.125);
  if (argc > 2)
  {
    filter->SetNumberOfIterations(atoi(argv[2]));
  }
  if (argc > 3)
  {
    filter->SetConductanceParameter(atof(argv[3]));
  }

  // 4) Cast the Vector image to an RGB image for display
  using AdaptorOutputType = itk::VectorToRGBImageAdaptor<FloatImageType>;
  auto adaptOutput = AdaptorOutputType::New();
  adaptOutput->SetImage(filter->GetOutput());
  using CastOutputType = itk::CastImageFilter<AdaptorOutputType, RGBImageType>;
  auto castOutput = CastOutputType::New();
  castOutput->SetInput(adaptOutput);

  // 5) Display the input and smoothed images
#ifdef ENABLE_QUICKVIEW
  QuickView viewer;
  viewer.AddRGBImage(input.GetPointer(), true, itksys::SystemTools::GetFilenameName(inputFileName));

  std::stringstream desc;
  desc << "VectorCurvatureAnisotropicDiffusionImageFilter\niterations: " << filter->GetNumberOfIterations()
       << " conductance: " << filter->GetConductanceParameter();
  viewer.AddRGBImage(castOutput->GetOutput(), true, desc.str());

  viewer.Visualize();
#endif

  return EXIT_SUCCESS;
}
