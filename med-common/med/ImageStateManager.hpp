#ifndef IMAGESTATEMANAGER_H
#define IMAGESTATEMANAGER_H

#include "MedFlow_pri.h"
#include <map>
#include "common/common.h"

namespace med {

class ImageStateManager
{
public:
    ImageStateManager(){}
    ~ImageStateManager(){}

    void saveState(int type, ImageType::Pointer ptr){
        ImageState state;
        state.size = ptr->GetLargestPossibleRegion().GetSize();
        state.spacing = ptr->GetSpacing();
        state.origin = ptr->GetOrigin();
        state.direction = ptr->GetDirection();
        //
        m_stateMap[type] = std::move(state);
    }

    bool getImageState(int type, ImageState* out){
        auto it = m_stateMap.find(type);
        if(it != m_stateMap.end()){
            *out = it->second;
            return true;
        }
        return false;
    }
    bool removeState(int type, ImageState* out){
        auto it = m_stateMap.find(type);
        if(it != m_stateMap.end()){
            if(out){
                *out = it->second;
            }
            m_stateMap.erase(it);
            return true;
        }
        return false;
    }
private:
    std::map<int, ImageState> m_stateMap;
};

}

#endif // IMAGESTATEMANAGER_H
