#ifndef NIFTI_READER_HPP
#define NIFTI_READER_HPP

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinomialBlurImageFilter.h"

#include "itkImageRegionIterator.h"
#include <itkNiftiImageIO.h>

#include "base1/test_common.h"
#include "PerformanceHelper.h"

namespace h7 {

template<typename InputPixelType0 = short, unsigned int Dimension = 2>
class NiftiHelper{
public:
    using InputPixelType = InputPixelType0;
    using ImageType = itk::Image<InputPixelType, Dimension>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ImageIOType = itk::NiftiImageIO;

    auto read(const String& in_file){
        //
        //ReaderType::Pointer reader = ReaderType::New();
        auto reader = ReaderType::New();
        ImageIOType::Pointer niftiIO = ImageIOType::New();
        reader -> SetImageIO(niftiIO);
        reader -> SetFileName(in_file.data());
        reader -> Update();
        return reader;
    }

private:

};

static inline itk::Image<double,3>::Pointer read_nii_image(const char* file_path) {
    using ImageType = itk::Image<double, 3>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ImageIoType = itk::NiftiImageIO;

    ReaderType::Pointer reader = ReaderType::New();
    ImageIoType::Pointer niftiIO = ImageIoType::New();
    reader->SetImageIO(niftiIO);

    reader->SetFileName(file_path);
    reader->Update();

    ImageType::Pointer image = reader->GetOutput();
    return image;
}

static inline itk::Image<unsigned char,3>::Pointer read_nii_image_u8(const char* file_path) {
    using ImageType = itk::Image<unsigned char, 3>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ImageIoType = itk::NiftiImageIO;

    ReaderType::Pointer reader = ReaderType::New();
    ImageIoType::Pointer niftiIO = ImageIoType::New();
    reader->SetImageIO(niftiIO);

    reader->SetFileName(file_path);
    reader->Update();

    ImageType::Pointer image = reader->GetOutput();
    return image;
}

template <typename TImage>
void
DeepCopy(typename TImage::Pointer input, typename TImage::Pointer output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImage>      outputIterator(output, output->GetLargestPossibleRegion());

  while (!inputIterator.IsAtEnd())
  {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
  }
}

static void print_vals_u8_3(itk::Image<unsigned char, 3>::Pointer p){
    auto size = p->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
    using IndexType = itk::Index<3>;
    IndexType index;
    std::set<int> set;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val = p->GetPixel(index);
                set.insert(val);
            }
        }
    }
    auto it = set.begin();
    while(it != set.end()){
        printf("set val = %d\n", *it);
        ++it;
    }
    //0: not, 1: 结节， 3：良性, 2 :恶性
}

}

#endif // NIFTI_READER_HPP
