
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include "itkImageRegionIterator.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkMeanImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"

#include "../nifti_reader.hpp"

#include <set>

#define Dimension 3
using ImageType = itk::Image<unsigned char, Dimension>;
using IndexType = itk::Index<Dimension>;

using OutputPixelType = unsigned char;
using InputImageType = itk::Image<OutputPixelType, Dimension>;
using OutputImageType = itk::Image<OutputPixelType, Dimension>;
using OutputImageType2 = itk::Image<float, Dimension>;
//split
static void split_retain_nodules(ImageType::Pointer p, int nodule_val,
                                 ImageType::Pointer out);
static void retain_val(ImageType::Pointer p, int retain_val);

static void smooth(const String& name,ImageType::Pointer p, int repeat_count);
static void smooth_gaussian(const String& name,ImageType::Pointer p, float sigma);

//1, 分离。 2，平滑。 3，融合（or 配准）
int test_med(int argc, char * argv[]){
    using namespace std;
    const char* filename = "/media/heaven7/h7/3d_recreate/1-1_mask.nii";
    if(argc > 1){
        filename = argv[1];
    }
    float val = 1.5f;
    if(argc > 2){
        val = stod(argv[2]);
    }
    //
    ImageType::Pointer img1 = h7::read_nii_image_u8(filename);
    ImageType::Pointer img2 = ImageType::New();
    //h7::DeepCopy<ImageType>(img1, img2);
    //1
    split_retain_nodules(img1, 1, img2);
    //itk::WriteImage(img1, "img1.nii");
    //itk::WriteImage(img2, "img2.nii");
    printf("split_retain_nodules done.\n");
    //retain_val(img1, 1);
    //retain_val(img2, 3);

    //2 . 结节平滑问题。若和腺体混在一起，则结节会被削峰。中值滤波还可以
    //      查找其它平滑方案？
    //smooth("img1_1.nii", img1, val); //wrong result
    //smooth_gaussian("img1_1.nii", img1, val);//nodule
    //itk::WriteImage(img1, "img1_1.nii");
    //printf("img1_1 done.\n");

   // smooth("img2_1.nii",img2, 6); //thy
    smooth("img2_1_100.nii",img2, 50); //thy
    //printf("img2 done.\n");

    //
    //itk::WriteImage(img1, "img1.nii");
    //itk::WriteImage(img2, "img2.nii");
    return 0;
}

void split_retain_nodules(ImageType::Pointer p_nodule, int nodule_val,
                          ImageType::Pointer out){
    out->SetRegions(p_nodule->GetLargestPossibleRegion());
    out->Allocate();
    //
    auto size = p_nodule->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
    IndexType index;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val = p_nodule->GetPixel(index);
                if(val == nodule_val){
                    out->SetPixel(index, 0);
                }else{
                    p_nodule->SetPixel(index, 0);
                    out->SetPixel(index, val);
                }
            }
        }
    }
}
void retain_val(ImageType::Pointer p, int retain_val){
    auto size = p->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
    IndexType index;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val = p->GetPixel(index);
                if(val != retain_val && val != 0){
                    p->SetPixel(index, 0);
                }
            }
        }
    }
}
static void smooth(const String& name,ImageType::Pointer p, int repeat_count){
    using FilterType = itk::BinomialBlurImageFilter<InputImageType, OutputImageType2>;
    auto filter = FilterType::New();
    filter->SetInput(p);
    filter->SetRepetitions(repeat_count);
    filter->SetNumberOfWorkUnits(8);
    filter->Update();


    itk::WriteImage(filter->GetOutput(), name.data());
}

//DiscreteGaussianImageFilter: 平滑后，结节不见了。
//GradientAnisotropicDiffusionImageFilter and MeanImageFilter both ok.
//SmoothingRecursiveGaussianImageFilter, DiscreteGaussianImageFilter not
static void smooth_gaussian(const String& name,ImageType::Pointer p, float sigma){
//    using FilterType = itk::SmoothingRecursiveGaussianImageFilter<ImageType, OutputImageType2>;
//    auto filter = FilterType::New();
//    filter->SetSigma(sigma);

    //    using FilterType = itk::MeanImageFilter<ImageType, OutputImageType2>;
    //    auto filter = FilterType::New();
    //    FilterType::InputSizeType radius;
    //    radius.Fill(sigma);
    //    filter->SetRadius(radius); //3

//     using FilterType = itk::DiscreteGaussianImageFilter<ImageType, OutputImageType2>;
//     auto filter = FilterType::New();
//     filter->SetVariance(sigma);

     using FilterType = itk::GradientAnisotropicDiffusionImageFilter<InputImageType, OutputImageType2>;
     auto filter = FilterType::New();
     filter->SetNumberOfIterations(6);
     filter->SetTimeStep(p->GetTimeStamp());//0.125
     filter->SetConductanceParameter(sigma); //3 is good

     filter->SetInput(p);
     filter->Update();

     itk::WriteImage(filter->GetOutput(), name.data());
}

int test_med_color(int argc, char* argv[]){
    String fn_thy = "img2_1_100.nii";
    //String nodule = "";
    ImageType::Pointer ptr = h7::read_nii_image_u8(fn_thy.data());

    auto size = ptr->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
    IndexType index;
    for(int i = 0 ; i < cc1 ; ++i){
        index[0] = i;
        for(int j = 0 ; j < cc2 ; ++j){
            index[1] = j;
            for(int k = 0 ; k < cc3 ; ++k){
                index[2] = k;
                int val = ptr->GetPixel(index);
                if(val != 0){
                    ptr->SetPixel(index, 3);
                }
            }
        }
    }
    itk::WriteImage(ptr, "img2_1_100_adjust.nii");
    return 0;
}
