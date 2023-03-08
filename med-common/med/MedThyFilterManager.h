#pragma once

#include "MedFlow_pri.h"
#include <map>

#include "utils/PerformanceHelper.h"

namespace med {

class MedThyFilterManager
{
public:
    MedThyFilterManager(){
        register_all();
    };

    void register_filter(int type, Func_Filter fiter){
        m_map[type] = fiter;
    }

    ImgPtr apply_filter(int type, ImgPtr src, med::MedThy_Param* p){
        std::string tystr = filter_type_to_str(type);
        LOGD("apply_filter >> start -> %s\n", tystr.data());
        auto it = m_map.find(type);
        if(it != m_map.end()){
            std::string out;
            m_PH.begin();
            ImgPtr ret = it->second(src, p);
            m_PH.printTo("apply_filter", out);
            LOGD("%s\n", out.data());
            return ret;
        }
        return nullptr;
    }
    static med::MedThyFilterManager* get(){
        static med::MedThyFilterManager filterM;
        return &filterM;
    }
private:
    std::map<int, Func_Filter> m_map;
    h7::PerfHelper m_PH;

    void register_all();
};


}
