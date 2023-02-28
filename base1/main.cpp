
#include "test_common.h"

extern void test_rgb_to_vtk();
extern void test_rgb_itk();
extern void test_vtk_to_itk();

extern int test_registration_mean_square();
extern int test_registration_global_two_imgs();
extern int test_registration_match_feature_points();
extern int test_multi_resotion();

extern int test_itk_to_vtk(int argc, char* argv[]);
extern int test_multi_model_mutual_Info(int argc, char* argv[]);
extern int test_regi_landmarks(int argc, char* argv[]);
extern int test_smoothing_binomial_blur(int argc, char* argv[]);
extern int test_high_derivative(int argc, char* argv[]);
extern int test_curvature_anisotropic(int argc, char* argv[]);
extern int test_curvature_anisotropic_nifti(int argc, char* argv[]);
extern int test_curvature_flow(int argc, char* argv[]);
extern int test_curvature_preserve_edge(int argc, char* argv[]);

extern int test_print_nifti(int argc, char* argv[]);
extern int test_med(int argc, char* argv[]);
extern int test_med2(int argc, char* argv[]);
extern int test_med_regi(int argc, char* argv[]);
//
typedef int (*Ptr_Func)(int argc, char* argv[]);

static std::map<String, Ptr_Func> sFuncs = {
    {"test_multi_model_mutual_Info", test_multi_model_mutual_Info},
    {"test_regi_landmarks", test_regi_landmarks},
    {"test_smoothing_binomial_blur", test_smoothing_binomial_blur},
    {"test_high_derivative", test_high_derivative},
    {"test_curvature_anisotropic", test_curvature_anisotropic},
    {"test_curvature_anisotropic_nifti", test_curvature_anisotropic_nifti},
    {"test_curvature_flow", test_curvature_flow},
    {"test_curvature_preserve_edge", test_curvature_preserve_edge},

    {"test_med", test_med},
    {"test_med2", test_med2},
    {"test_med_regi", test_med_regi},
    {"test_itk_to_vtk", test_itk_to_vtk},
    {"test_print_nifti", test_print_nifti},
};

//all itk examples in : https://examples.itk.org/src/core/common/applyafilteronlytoaspecifiedregionofanimage/documentation
int main(int argc, char* argv[]){
    setbuf(stdout, NULL);
    if(argc >= 3){
        //--mode xxx
        String mode_prefix = "--mode";
        if(mode_prefix == argv[1]){
            Ptr_Func ptr = sFuncs[String(argv[2])];
            //
            std::vector<String> params;
            params.push_back(argv[0]);

            for(int i = 3 ; i < argc ; i ++){
                params.push_back(argv[i]);
            }
            if(ptr != nullptr){
                char** param_ptrs = (char**)malloc(params.size() * sizeof (char*));
                int c = params.size();
                for(int i = 0 ; i < c ; ++i){
                    //printf("p(%d) = %s\n", i, params[i].data());
                    param_ptrs[i] = params[i].data();
                }
                int ret = ptr(c, param_ptrs);
                free(param_ptrs);
                return ret;
            }
        }
    }
    test_med(argc, argv);
    //test_med2(argc, argv);
    //test_print_nifti(argc, argv);
    //test_med_regi(argc, argv);

    // test_rgb_to_vtk();
    // test_vtk_to_itk();
    // test_registration_global_two_imgs();
    // test_registration_match_feature_points();
    //test_multi_resotion();

     //./test1 --mode test_high_derivative 1.png
    //String OUT_FILE="/home/heaven7/heaven7/libs/ITK/sample/test_out/cur_ani_nifti";
    //String IN_FILE = "/media/heaven7/h7/3d_recreate/1-1_img.nii";
//    std::vector<String> params = {
//        String("./test1"),
//        "--mode",
//        "test_curvature_anisotropic_nifti",
//        "/media/heaven7/h7/3d_recreate/1-1_img.nii",
//        "/home/heaven7/heaven7/libs/ITK/sample/test_out/cur_ani_nifti",
//        "10",
//        "0.0625",
//        "3.0",
//        ".nii"
//    };
//    char** param_ptrs = (char**)malloc(params.size() * sizeof (char*));
//    int c = params.size();
//    for(int i = 0 ; i < c ; ++i){
//        param_ptrs[i] = params[i].data();
//    }
//    int ret = main(c, param_ptrs);
//    free(param_ptrs);
//    return ret;
    return 0;
}
/*
 BMPImageIO
    BioRadImageIO
    Bruker2dseqImageIO
    GDCMImageIO
    GE4ImageIO
    GE5ImageIO
    GiplImageIO
    HDF5ImageIO
    JPEGImageIO
    JPEG2000ImageIO
    LSMImageIO
    MINCImageIO
    MRCImageIO
    MetaImageIO
    NiftiImageIO
    NrrdImageIO
    PNGImageIO
    StimulateImageIO
    TIFFImageIO
    VTKImageIO

 */
