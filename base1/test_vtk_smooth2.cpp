#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkInteractionStyle);

#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkButterflySubdivisionFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTriangleFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkSmoothPolyDataFilter.h>
#include "utils/PerformanceHelper.h"

static vtkSmartPointer<vtkActor> gen_actor(vtkSmartPointer<vtkPolyData>);
static vtkSmartPointer<vtkActor> gen_actor2(vtkSmartPointer<vtkPolyData>);

//test thy data.
//crash: 32G内存, 700W+三角面数(300W+顶点).
int test_vtk_smooth2(int argc, char* argv[])
{
    //读数据
    vtkSmartPointer<vtkPolyDataReader> reader =
        vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName("thy_vtk.vtk");
    reader->Update();

    vtkSmartPointer<vtkPolyData> orig = reader->GetOutput();
    std::cout << "original" << "-----------------------------" << std::endl;
    std::cout << "模型点数为： " << orig->GetNumberOfPoints() << std::endl;
    std::cout << "模型面数为： " << orig->GetNumberOfPolys() << std::endl;

    vtkSmartPointer<vtkPolyDataMapper> origMapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    origMapper->SetInputData(orig);
    vtkSmartPointer<vtkActor> origActor =
        vtkSmartPointer<vtkActor>::New();
    origActor->SetMapper(origMapper);

    vtkSmartPointer<vtkActor> loopActor = gen_actor2(orig);
    //
    double ltView[4] = { 0, 0, 0.5, 0.5 };
    double rtView[4] = { 0.5, 0, 1, 0.5 };
    double lbView[4] = { 0, 0.5, 0.5, 1 };
    double rbView[4] = { 0.5, 0.5, 1, 1 };

    vtkSmartPointer<vtkRenderer> origRender =
        vtkSmartPointer<vtkRenderer>::New();
    origRender->SetViewport(ltView);
    origRender->AddActor(origActor);
    origRender->SetBackground(1, 0, 0);

    vtkSmartPointer<vtkRenderer> loopRender =
        vtkSmartPointer<vtkRenderer>::New();
    loopRender->SetViewport(lbView);
    loopRender->AddActor(loopActor);
    loopRender->SetBackground(0, 0, 1);

    //
    vtkSmartPointer<vtkRenderWindow> rw =
        vtkSmartPointer<vtkRenderWindow>::New();
    rw->AddRenderer(origRender);
   // rw->AddRenderer(linearRender);
    rw->AddRenderer(loopRender);
   // rw->AddRenderer(butterflyRender);
    rw->SetSize(800, 600);
    rw->SetWindowName("PolyData Subdivision");

    origRender->GetActiveCamera()->SetPosition(0, -1, 0);
    origRender->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    origRender->GetActiveCamera()->SetViewUp(0, 0, 1);
    origRender->GetActiveCamera()->Azimuth(30);
    origRender->GetActiveCamera()->Elevation(30);
    origRender->ResetCamera();//刷新照相机
    //linearRender->SetActiveCamera(origRender->GetActiveCamera());
    loopRender->SetActiveCamera(origRender->GetActiveCamera());
    //butterflyRender->SetActiveCamera(origRender->GetActiveCamera());

    vtkSmartPointer<vtkRenderWindowInteractor> rwi =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    rwi->SetRenderWindow(rw);
    rwi->Start();
    rw->Render();

    return 0;
}

vtkSmartPointer<vtkActor> gen_actor2(vtkSmartPointer<vtkPolyData> _orig){
    h7::PerfHelper ph;
    ph.begin();
    vtkSmartPointer<vtkSmoothPolyDataFilter> filter =
            vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    filter->SetInputData(_orig);
    filter->SetNumberOfIterations(1000);
    filter->Update();
    ph.print("vtkSmoothPolyDataFilter");

    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(filter->GetOutput());
    vtkSmartPointer<vtkActor> loopActor =
        vtkSmartPointer<vtkActor>::New();
    loopActor->SetMapper(mapper);
    return loopActor;
}

vtkSmartPointer<vtkActor> gen_actor(vtkSmartPointer<vtkPolyData> _orig){
    //crash: 32G内存, 700W+三角面数(300W+顶点).

    vtkSmartPointer<vtkTriangleFilter> tri_filter = vtkSmartPointer<vtkTriangleFilter>::New();
    tri_filter->SetInputData(_orig);
    tri_filter->Update();
    vtkSmartPointer<vtkPolyData> orig = tri_filter->GetOutput();
    std::cout << "vtkTriangleFilter 模型点数为： " << orig->GetNumberOfPoints()
              << std::endl;
    std::cout << "vtkTriangleFilter 模型面数为： " << orig->GetNumberOfPolys()
              << std::endl;

    //Clean the polydata so that the edges are shared !
    vtkSmartPointer<vtkCleanPolyData> clean_pld =
        vtkSmartPointer<vtkCleanPolyData>::New();
    clean_pld->SetInputData(orig);
    clean_pld->Update();

    //线性网格细分滤波器
//    vtkSmartPointer<vtkLinearSubdivisionFilter> loop =
//        vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
//    loop->SetInputData(orig);
//    loop->SetNumberOfSubdivisions(4);//细化的次数 四的倍数 16
//    loop->Update();

//    vtkSmartPointer<vtkPolyData> linearInfo = linear->GetOutput();
//    std::cout << "linear" << "-----------------------" << std::endl;
//    std::cout << "模型点数为： " << linearInfo->GetNumberOfPoints() << std::endl;
//    std::cout << "模型面数为： " << linearInfo->GetNumberOfPolys() << std::endl;

    //Loop网格细分滤波器
    vtkSmartPointer<vtkLoopSubdivisionFilter> loop =
        vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
    loop->SetInputData(clean_pld->GetOutput());
    loop->SetNumberOfSubdivisions(3);
    loop->Update();

    vtkSmartPointer<vtkPolyData> loopInfo = loop->GetOutput();
    std::cout << "loop" << "-----------------------" << std::endl;
    std::cout << "模型点数为： " << loopInfo->GetNumberOfPoints() << std::endl;
    std::cout << "模型面数为： " << loopInfo->GetNumberOfPolys() << std::endl;

    //butterfly网格细分滤波器
//    vtkSmartPointer<vtkButterflySubdivisionFilter> loop =
//        vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
//    loop->SetInputData(orig);
//    loop->SetNumberOfSubdivisions(4);
//    loop->Update();

//    vtkSmartPointer<vtkPolyData> butterflyInfo = loop->GetOutput();
//    std::cout << "butterfly" << "-----------------------" << std::endl;
//    std::cout << "模型点数为： " << butterflyInfo->GetNumberOfPoints() << std::endl;
//    std::cout << "模型面数为： " << butterflyInfo->GetNumberOfPolys() << std::endl;

    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(loop->GetOutput());
    vtkSmartPointer<vtkActor> loopActor =
        vtkSmartPointer<vtkActor>::New();
    loopActor->SetMapper(mapper);
    return loopActor;
}
