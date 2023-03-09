#pragma once

#include "MedFlow_ctx.h"
#include <string>

namespace med {
typedef struct _MedThyContext MedThyContext;
}

namespace med {
    enum{
        kITKFLOW_NONE = 0,
        kITKFLOW_NODULE_BAD  = 1,
        kITKFLOW_NODULE_GOOD = 2,
        kITKFLOW_THY = 3,
    };

    struct InputParams{
        MedThy_Param thy;
        MedThy_Param nodule_good;
        MedThy_Param nodule_bad;
    };

    class ImageStateManager;

    class MedThyFlow{
    public:
        using CString = const std::string&;

        MedThyFlow(const InputParams& lfp);
        ~MedThyFlow();
        InputParams& getInputParams(){
            return m_params;
        }

        void read_nifti(CString path);
        //like nodule and thy
        void split_data();

        void resamples(int type, float* spaces, int space_count = 3);

        void smooth_thy(int filter_type, bool onlyTwoTypeValue);

        void smooth_nodules(int filter_type, bool onlyTwoTypeValue);

        void merge();

        void save_thy(CString filename);
        void save_nodules(CString fn_good, CString fn_bad);
        void save_merge_result(CString filename);
        //---- ---
        void load_nifti(int type,CString filename);
        void load_thy(CString filename){
            load_nifti(kITKFLOW_THY, filename);
        }
        void save_imageState(int type);
        void restore_imageState(int type);
    private:
        static void apply_resample(MedThyContext* ctx, int type, void* state);

    private:
        MedThyContext* m_ctx {nullptr};
        ImageStateManager* m_stateM {nullptr};
        InputParams m_params;
    };

}
