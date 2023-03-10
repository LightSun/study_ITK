#ifndef VTK_CTX_H
#define VTK_CTX_H

#include <vector>
#include <string>

template <class T>
class vtkSmartPointer;
class vtkActor;

namespace h7 {
    enum{
        kBackground_change_R = 1,
        kBackground_change_G,
        kBackground_change_B,
        kBackground_change_A,
        kBackground_change_RGBA,
    };
    struct VTKFlow_Param{
        int bg_change_mode {kBackground_change_RGBA};
        int window_width {800};
        int window_height {600};
        std::string window_name {"demo"};
    };

}

#endif // VTK_CTX_H
