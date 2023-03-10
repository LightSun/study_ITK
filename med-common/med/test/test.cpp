#include "../MedFlow.hpp"
#include "test.h"

#include "utils/ConfigUtils.h"
#include "utils/ArgsParser.h"
#include "utils/FileUtils.h"

#define DEF_MAP_ELE(n) {#n, med::kType_##n}
static std::map<String, int> sFilterMap = {
    DEF_MAP_ELE(BinomialBlur),
    DEF_MAP_ELE(SmoothingRecursiveGaussian),
    DEF_MAP_ELE(RecursiveGaussian),
    DEF_MAP_ELE(Mean),
    DEF_MAP_ELE(Median),
    DEF_MAP_ELE(DiscreteGaussian),
    DEF_MAP_ELE(GradientAnisotropicDiffusion),
    DEF_MAP_ELE(CurvatureAnisotropicDiffusion),
    DEF_MAP_ELE(CurvatureFlow),
    DEF_MAP_ELE(Bilateral),
    DEF_MAP_ELE(VTK_Smooth)
};
#undef DEF_MAP_ELE

using ConfigUtils = h7::ConfigUtils;
using Properties = h7::Properties;

namespace test {
void loadConfig(CString config_file, FlowParam* fp, MedThy_Param* tp_thy,
                MedThy_Param* tp_nodule){
    Properties prop;
    ConfigUtils::loadProperties(config_file, prop.m_map);
    ConfigUtils::resolveProperties(prop.m_map);
    //flow
    fp->nifti_path = prop.getString("flow::nifti_path");
    fp->thy_filter_type = sFilterMap[prop.getString("flow::thy_filter_type")];
    fp->nodules_filter_type = sFilterMap[prop.getString("flow::nodules_filter_type")];
    fp->thy_binary = prop.getBool("flow::thy_binary");
    fp->nodules_binary = prop.getBool("flow::nodules_binary");

    fp->save_thy_name = prop.getString("flow::save_thy_name");
    fp->save_merge_name = prop.getString("flow::save_merge_name");
    prop.getVector("flow::save_nodules_name",fp->save_nodules_name);
    //thy
    tp_thy->thread_count = prop.getInt("thy::thread_count", tp_thy->thread_count);
    tp_thy->repeat_count = prop.getInt("thy::repeat_count", tp_thy->repeat_count);
    tp_thy->direction = prop.getInt("thy::conductance", tp_thy->direction);

    tp_thy->time_step = prop.getFloat("thy::time_step", tp_thy->time_step);
    tp_thy->radius = prop.getFloat("thy::radius", tp_thy->radius);
    tp_thy->sigma = prop.getFloat("thy::sigma", tp_thy->sigma);
    tp_thy->variance = prop.getFloat("thy::variance", tp_thy->variance);
    tp_thy->conductance = prop.getFloat("thy::conductance", tp_thy->conductance);
    tp_thy->directionTolerance = prop.getFloat("thy::directionTolerance",
                                               tp_thy->directionTolerance);
    tp_thy->poly2img_spacing = prop.getFloat("thy::poly2img_spacing",
                                               tp_thy->poly2img_spacing);

    tp_thy->secondOrder = prop.getBool("thy::secondOrder");
    tp_thy->name_fmt = prop.getString("thy::name_fmt");
    tp_thy->spacing = prop.getInt("thy::spacing", tp_thy->spacing);
    //nodule
    tp_nodule->thread_count = prop.getInt("nodules::thread_count", tp_nodule->thread_count);
    tp_nodule->repeat_count = prop.getInt("nodules::repeat_count", tp_nodule->repeat_count);
    tp_nodule->direction = prop.getInt("nodules::conductance", tp_nodule->direction);

    tp_nodule->time_step = prop.getFloat("nodules::time_step", tp_nodule->time_step);
    tp_nodule->radius = prop.getFloat("nodules::radius", tp_nodule->radius);
    tp_nodule->sigma = prop.getFloat("nodules::sigma", tp_nodule->sigma);
    tp_nodule->variance = prop.getFloat("nodules::variance", tp_nodule->variance);
    tp_nodule->conductance = prop.getFloat("nodules::conductance", tp_nodule->conductance);
    tp_nodule->directionTolerance = prop.getFloat("nodules::directionTolerance",
                                               tp_nodule->directionTolerance);
    tp_nodule->poly2img_spacing = prop.getFloat("thy::poly2img_spacing",
                                               tp_nodule->poly2img_spacing);

    tp_nodule->secondOrder = prop.getBool("nodules::secondOrder");
    tp_nodule->name_fmt = prop.getString("nodules::name_fmt");
    tp_nodule->spacing = prop.getInt("nodules::spacing", tp_nodule->spacing);
}

//return next start position
static int fmt_parse0(CString fmt, std::vector<FmtItem>& out, int start_pos){
    int index, index2;
    index = fmt.find("<", start_pos);
    if(index < 0){
        return -1;
    }
    index2 = fmt.find(">", index + 1);
    if(index2 < 0){
        return -1;
    }
    FmtItem item;
    item.name = fmt.substr(index + 1, index2 - index - 1);
    item.startPos = index;
    out.push_back(std::move(item));
    return index + item.name.length() + 2;
}
void fmt_parse(CString fmt, std::vector<FmtItem>& out){
    int start_pos = 0;
    do{
        start_pos = fmt_parse0(fmt, out, start_pos);
    }while (start_pos >= 0);
}

 //return the char count of add
int fmt_replace(String& src, const FmtItem& fmt, MedThy_Param* p){
    auto str = med_thy_param_format(p, fmt.name);
    //printf("fmt_replace >> name(%s) = %s \n", fmt.name.data(), str.data());
    src = src.replace(fmt.startPos, fmt.name.length() + 2, str);
    return str.length() - (fmt.name.length() + 2);
}

}
