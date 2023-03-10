#ifndef ITKFLOW_CTX_H
#define ITKFLOW_CTX_H

#include <string>
#include <map>

namespace med {

struct MedThy_Param{
    int spacing {0};
    int thread_count {8};
    int repeat_count {1}; // the iterate count
    float sigma {3};

    float radius {3};
    float variance {3};
    float conductance {3};
    float time_step {0};
    float poly2img_spacing {1};

    int direction {-1}; //0 means 'x-axis', 1 mean y-axis
    float directionTolerance {-1};
    bool secondOrder {false};
    //the name format of gen file.
    std::string name_fmt;
};

typedef enum{
    kType_BinomialBlur = 1,
    kType_SmoothingRecursiveGaussian,
    kType_RecursiveGaussian,
    kType_Mean,
    kType_Median,
    kType_DiscreteGaussian,
    kType_GradientAnisotropicDiffusion,
    kType_CurvatureAnisotropicDiffusion,
    kType_CurvatureFlow,
    kType_Bilateral,
    kType_VTK_Smooth,
}MedThy_FilterType;

static inline std::string filter_type_to_str(int type){
#define __FT_CASE_STR(n) case kType_##n: return #n;
    switch (type) {
        __FT_CASE_STR(BinomialBlur)
        __FT_CASE_STR(SmoothingRecursiveGaussian)
        __FT_CASE_STR(RecursiveGaussian)
        __FT_CASE_STR(Mean)
        __FT_CASE_STR(Median)
        __FT_CASE_STR(DiscreteGaussian)
        __FT_CASE_STR(GradientAnisotropicDiffusion)
        __FT_CASE_STR(CurvatureAnisotropicDiffusion)
        __FT_CASE_STR(CurvatureFlow)
        __FT_CASE_STR(Bilateral)
        __FT_CASE_STR(VTK_Smooth)
    }
#undef __FT_CASE_STR
    return "unknown";
}

static inline std::string med_thy_param_format(MedThy_Param* p,
                                              const std::string& name){
    std::map<std::string, std::string> map;
#define __REG_X(n) map[#n]=std::to_string(p->n)
    __REG_X(repeat_count);
    __REG_X(sigma);
    __REG_X(radius);
    __REG_X(variance);
    __REG_X(conductance);
    __REG_X(time_step);
    __REG_X(direction);
    __REG_X(directionTolerance);
    __REG_X(secondOrder);
    __REG_X(poly2img_spacing);
#undef __REG_X
    return map[name];
}

}

#endif // ITKFLOW_CTX_H
