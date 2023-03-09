#include "../MedFlow.hpp"
#include <map>
#include "utils/ConfigUtils.h"
#include "utils/ArgsParser.h"
#include "utils/FileUtils.h"
#include "common/logger.h"
#include "test.h"

using namespace h7;

static void do_flow(med::MedThyFlow& flow,test::FlowParam& fp, CString dir);
static void do_flow_simple(med::MedThyFlow& flow,test::FlowParam& fp, CString dir);

//--out_dir xxx --config xxx.prop
int main(int argc, char* argv[]){
    setbuf(stdout, NULL);
    if(argc < 5){
        String dir = "/home/heaven7/heaven7/study/github/mine/"
                     "build-study_ITK-Desktop_Qt_5_14_2_GCC_64bit-Debug";
        String order = "7";
        std::vector<String> params = {
               argv[0],
               "--out_dir",
               dir + "/test/test" + order,
               "--config",
               dir + "/test/test" + order + "/test.prop",
        };
        ArgsParser ap(params);
        return ap.run(main);
    }
    h7::LoggerParam mlp;
    mlp.name = "med_test";
    mlp.level = "DEBUG";
    h7::initLogger(mlp);
    //
    Properties prop = ArgsParser(argc, argv).asProperty();
    String dir = prop.getString("out_dir");
    FileUtils::mkdir(dir);
    String config = prop.getString("config");
    //load
    test::FlowParam fp;
    med::InputParams ps;
    test::loadConfig(config, &fp, &ps.thy, &ps.nodule_good);
    ps.nodule_bad = ps.nodule_good;
    //run
    med::MedThyFlow flow(ps);
    //
    //do_flow(flow, fp, dir);
    do_flow_simple(flow, fp, dir);
    return 0;
}

//------------------
static void do_flow_simple(med::MedThyFlow& flow,test::FlowParam& fp, CString dir){
    String thy_name = fp.save_thy_name + ".nii";
    //by test: down-sample spacing should be 2.
    int space = fp.spacing;
    flow.load_thy(thy_name);
    //
    flow.save_imageState(med::kITKFLOW_THY);
    float spaces[3];
    spaces[0] = space;
    spaces[1] = space;
    spaces[2] = space;
    //down sample
    flow.resamples(med::kITKFLOW_THY, spaces);

    //flow.save_thy(fp.save_thy_name + "_resample4.nii"); //temp

    //smooth
    flow.smooth_thy(fp.thy_filter_type, fp.thy_binary);

    //restore image state.
    flow.restore_imageState(med::kITKFLOW_THY);//ok
//  flow.resamples(med::kITKFLOW_THY, spaces);//spaces[x,y, z] = 0.5f ok

    //mean
    std::vector<test::FmtItem> fmts;
    String fmt = flow.getInputParams().thy.name_fmt;
    if(!fmt.empty()){
        test::fmt_parse(fmt, fmts);
        int delta = 0;
        for(int i = 0 ; i < (int)fmts.size() ; ++i){
             delta += test::fmt_replace(fmt, fmts[i], &flow.getInputParams().thy);
             if(i + 1 < (int)fmts.size()){
                fmts[i + 1].startPos += delta;
             }
        }
        h7_logd("handle_fmt >> %s -> %s\n",
                flow.getInputParams().thy.name_fmt.data(), fmt.data());
    }
    String space_str = "_" + std::to_string(space);
    String dst = dir + "/" + fp.save_thy_name + fmt + space_str + ".nii";
    flow.save_thy(dst);
}


static void do_flow(med::MedThyFlow& flow,test::FlowParam& fp, CString dir){
    flow.read_nifti(fp.nifti_path);
    flow.split_data();
    //TODO
    {
        flow.save_thy(fp.save_thy_name + ".nii");
        String g_file = fp.save_nodules_name[0] + ".nii";
        String b_file = fp.save_nodules_name[1] + ".nii";
        flow.save_nodules(g_file, b_file);
    }

    flow.smooth_thy(fp.thy_filter_type, fp.thy_binary);
    if(!fp.save_thy_name.empty()){
        String dst = dir + "/" + fp.save_thy_name + ".nii";
        flow.save_thy(dst);
    }
    flow.smooth_nodules(fp.nodules_filter_type, fp.nodules_binary);
    if(fp.save_nodules_name.size() >= 2){
        String g_file = dir + "/" + fp.save_nodules_name[0] + ".nii";
        String b_file = dir + "/" + fp.save_nodules_name[1] + ".nii";
        flow.save_nodules(g_file, b_file);
    }
    flow.merge();
    if(!fp.save_merge_name.empty()){
        String dst = dir + "/" + fp.save_merge_name + ".nii";
        flow.save_merge_result(dst);
    }
}
