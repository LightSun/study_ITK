#ifndef IMAGESTATEMANAGER_H
#define IMAGESTATEMANAGER_H

#include "MedFlow_pri.h"
#include <map>
#include "common/common.h"

namespace med {

class ImageStateManager
{
public:
    struct ImageState{
        ImageType::SizeType size;
        ImageType::SpacingType spacing;
        ImageType::PointType origin;
        ImageType::DirectionType direction;

        void print(){
            std::string out;
            HFMT_BUF_128({
                         out += buf;
                         }, "size = %d, %d, %d\n",
                         (int)size[0], (int)size[1], (int)size[2]);
            HFMT_BUF_128({
                         out += buf;
                         }, "spacing = %d, %d, %d\n",
                         (int)spacing[0], (int)spacing[1], (int)spacing[2]);
            LOGD("ImageState >> %s", out.data());
        }
    };
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
