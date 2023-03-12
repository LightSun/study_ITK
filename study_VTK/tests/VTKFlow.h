#ifndef VTKFLOW_H
#define VTKFLOW_H

#include "VTKFlow_ctx.h"

namespace h7 {

class VTKFlow
{
public:
    using String = std::string;
    using CString = const std::string&;
    VTKFlow(const VTKFlow_Param& p = VTKFlow_Param()):m_params(p){
    }
    void show(const std::vector<vtkSmartPointer<vtkActor>>& actors);

    void show(vtkSmartPointer<vtkActor> actor);

    static vtkSmartPointer<vtkActor> newActor(vtkSmartPointer<vtkPolyData> mapper, CString color="");
private:
    VTKFlow_Param m_params;

    void gen_bg_color(double delta,double* rgba);
};

}

#endif // VTKFLOW_H
