
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include "../nifti_reader.hpp"

using namespace std;

#define DataType unsigned char
#define Dimension 3
using ImageType = itk::Image<DataType, Dimension>;
using IndexType = itk::Index<Dimension>;

static void print_vals(ImageType::Pointer p){
    auto size = p->GetLargestPossibleRegion().GetSize();
    int cc1 = size.GetElement(0);
    int cc2 = size.GetElement(1);
    int cc3 = size.GetElement(2);
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
    String val;
    while(it != set.end()){
        val += std::to_string(*it);
        val += ", ";
        ++it;
    }
     printf("set vals: %s\n", val.data());
    //0: not, 1: 结节， 3：良性, 2 :恶性
}

int test_print_nifti(int argc, char * argv[]){
    const char* filename = "/media/heaven7/h7/3d_recreate/1-1_mask.nii";
    if(argc > 1){
        filename = argv[1];
    }
    // 调用函数
    itk::Image<DataType, 3>::Pointer image = h7::read_nii_image_u8(filename);

    // 查看spacing
    itk::Image<DataType, 3>::SpacingType spacing = image->GetSpacing();
    cout << "spacing:\n " << spacing << endl;
    // 查看direction
    itk::Image<DataType, 3>::DirectionType direction = image->GetDirection();
    cout << "direction:\n " << direction << endl;
    // 查看origin
    itk::Image<DataType, 3>::PointType origin = image->GetOrigin();
    cout << "origin: \n" << origin << endl;
    // 查看大小: x,y,z
    itk::Image<DataType, 3>::SizeType size = image->GetLargestPossibleRegion().GetSize();
    cout << "size: \n" << size << endl;

    image->Print(std::cout);

    print_vals(image);
    return 0;
}
