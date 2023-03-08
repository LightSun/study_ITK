#ifndef NIFTISTATEMANAGER_H
#define NIFTISTATEMANAGER_H

#include <set>
#include "utils/FileIO.h"

namespace h7 {

    class NiftiStateManager_U8_3{
    public:
        void open(CString path){
            FileInput fis(path);
            MED_ASSERT(fis.is_open());
            std::vector<char> vec;
            fis.read2Vec(vec);
            MED_ASSERT(vec.size() % sizeof (long) == 0);
            int c = vec.size() / sizeof (long);
            for(int i = 0 ; i < c ; ++i){
                long val = *(long*)(vec.data() + i * sizeof (long));
                m_offsets.insert(val);
            }
        }
        void save(CString path){
            FileOutput fio;
            if(m_offsets.size() > 0){
                std::vector<long> vec(m_offsets.begin(), m_offsets.end());
                fio.open(path);
                MED_ASSERT(fio.is_open());
                fio.write(vec.data(), vec.size() * sizeof (long));
                fio.flush();
                fio.close();
            }else{
                printf("m_offsets is empty.\n");
            }
        }
        bool containsOffset(long offset){
            return m_offsets.find(offset) != m_offsets.end();
        }
        void addOffset(long offset){
            m_offsets.insert(offset);
        }
    private:
        std::set<long> m_offsets;
    };
}

#endif // NIFTISTATEMANAGER_H
