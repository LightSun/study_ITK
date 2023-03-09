#ifndef MED_TEST_H
#define MED_TEST_H

#include <string>
#include <map>
#include <vector>

#include "../MedFlow_ctx.h"

using String = std::string;
using CString = const std::string&;

typedef struct med::MedThy_Param MedThy_Param;

namespace test {

struct FlowParam{
    String nifti_path;
    int thy_filter_type;
    int nodules_filter_type;
    int spacing {2};
    bool thy_binary {true};
    bool nodules_binary {true};
    String save_thy_name;
    String save_merge_name;
    std::vector<String> save_nodules_name;
};

struct FmtItem{
    String name;
    int startPos;
};

void loadConfig(CString config_file, FlowParam* fp, MedThy_Param* tp_thy,
                    MedThy_Param* tp_nodule);

void fmt_parse(CString fmt, std::vector<FmtItem>& out);

 //return the char count of add
int fmt_replace(String& src, const FmtItem& fmt, MedThy_Param* p);

}

#endif // TEST_H
