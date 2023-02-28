#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "../nifti_reader.hpp"

//
//曲率各向异性扩散图像滤波器. 会变得光滑
//conductance: 会影响光滑度。越大，越光滑
int
test_curvature_anisotropic_nifti(int argc, char * argv[])
{
//  if (argc != 6)
//  {
//    std::cerr << "Usage: " << std::endl;
//    std::cerr << argv[0];
//    std::cerr << " <InputFileName> <OutputFileName>";
//    //default: 5  0.125  3.0
//    std::cerr << " <numberOfIterations> <timeStep> <conductance>";
//    std::cerr << std::endl;
//    return EXIT_FAILURE;
//  }

  String in_file = "/media/heaven7/h7/3d_recreate/1-1_img.nii";
  String out_file = "/home/heaven7/heaven7/libs/ITK/sample/test_out/cur_ani_nifti";
  if(argc > 1){
      in_file = argv[1];
  }
  if(argc > 2){
      out_file = argv[2];
  }
  int it_count = 1;
  if(argc > 3){
      it_count = std::stoi(argv[3]);
  }
  float timeStep = 0.0625;
  if(argc > 4){
      String str = argv[4];
      printf("timeStep = %s\n", str.data());
      timeStep = std::stod(str.data());
  }
  float conductance = 3.0f;
  if(argc > 5){
      String str = argv[5];
       printf("conductance = %s\n", str.data());
      conductance = std::stod(str.data());
  }
  //gen file suffix
  if(argc > 6){
      char buf[64];
      snprintf(buf , 64, "%.3f", conductance);
      out_file += "_" + std::to_string(it_count) + "_" + String(buf) + String(argv[6]);
  }else{
      out_file += ".nii";
  }
  //
  constexpr unsigned int Dimension = 3;

  typedef h7::NiftiHelper<float, Dimension> NHH;
  NHH nh;

  using InputImageType = NHH::ImageType;
  using OutputImageType = itk::Image<float, Dimension>;

  const auto input = nh.read(in_file);

  using FilterType = itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, OutputImageType>;
  auto filter = FilterType::New();
  filter->SetInput(input->GetOutput());
  filter->SetNumberOfIterations(it_count);
  filter->SetTimeStep(timeStep);
  filter->SetNumberOfWorkUnits(8);//thread count
  filter->SetConductanceParameter(conductance);
  //
  using WritePixelType = unsigned char;
  using WriteImageType = itk::Image<WritePixelType, Dimension>;;
  using RescaleType = itk::RescaleIntensityImageFilter<OutputImageType, WriteImageType>;
  auto rescaler = RescaleType::New();
  rescaler->SetInput(filter->GetOutput());
  rescaler->SetOutputMinimum(itk::NumericTraits<WritePixelType>::min());
  rescaler->SetOutputMaximum(itk::NumericTraits<WritePixelType>::max());

  try
  {
    itk::WriteImage(rescaler->GetOutput(), out_file.data());
  }
  catch (const itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
