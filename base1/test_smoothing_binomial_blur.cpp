#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinomialBlurImageFilter.h"

#include <itkNiftiImageIO.h>

#include "test_common.h"
#include "utils/PerformanceHelper.h"

/*
 * 二项式模糊图像滤波器
BinomialBlurImageFilter 沿每个维度计算最近邻平均值。
根据用户的指定，该过程会重复多次。
原则上，经过大量迭代后，结果将接近高斯卷积。
*/
int
test_smoothing_binomial_blur(int argc, char * argv[])
{
  //std::cerr << argv[0] << "  inputImageFile  outputImageFile  numberOfRepetitions" << std::endl;
  String in_file = argv[1];
  String out_file = argv[2];
  int repeat_count;
  if(argc > 3){
      repeat_count = std::stoi(argv[3]);
  }else{
      repeat_count = 10;
  }
  //check file suffix
  if(argc > 4){
      // gen_file_suffix is true;
      out_file += "_" + std::to_string(repeat_count) + String(argv[4]);
  }
  h7::PerfHelper ph;
  //
  using InputPixelType = short; //float
  constexpr unsigned int Dimension = 3;
  using ImageType = itk::Image<InputPixelType, Dimension>;
  using ReaderType = itk::ImageFileReader<ImageType>;
  using ImageIOType = itk::NiftiImageIO;
  ph.begin();
  //
  ReaderType::Pointer reader = ReaderType::New();
  ImageIOType::Pointer niftiIO = ImageIOType::New();
  reader -> SetImageIO(niftiIO);
  reader -> SetFileName(in_file.data());
  reader -> Update();
  ph.print("reader");
  std::cout << "reader done." << std::endl;

  using OutputPixelType = float;
  using InputImageType = itk::Image<InputPixelType, Dimension>;
  using OutputImageType = itk::Image<OutputPixelType, Dimension>;

  using FilterType = itk::BinomialBlurImageFilter<InputImageType, OutputImageType>;
  auto filter = FilterType::New();
  filter->SetInput(reader->GetOutput());
  filter->SetRepetitions(repeat_count);
  filter->SetNumberOfWorkUnits(8);
  filter->Update();
  ph.print("BinomialBlurImageFilter");
  std::cout << "BinomialBlurImageFilter done." << std::endl;
  itk::WriteImage(filter->GetOutput(), out_file.data());

//  using WritePixelType = unsigned char; //short
//  using WriteImageType = itk::Image<WritePixelType, Dimension>;
//  using RescaleFilterType = itk::RescaleIntensityImageFilter<OutputImageType, WriteImageType>;

//  auto rescaler = RescaleFilterType::New();
//  rescaler->SetOutputMinimum(0);
//  rescaler->SetOutputMaximum(255);//255?
//  rescaler->SetInput(filter->GetOutput());
//  rescaler->Update();

//  itk::WriteImage(rescaler->GetOutput(), out_file.data());

  return EXIT_SUCCESS;
}
