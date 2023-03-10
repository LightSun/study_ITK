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
#include <vtkImageResample.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkImageStencil.h>
#include <vtkInformation.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include "utils/PerformanceHelper.h"

extern vtkSmartPointer<vtkImageData> polyDataToImageData(vtkSmartPointer<vtkPolyData>);

static vtkSmartPointer<vtkActor> gen_actor(vtkSmartPointer<vtkPolyData>);
static vtkSmartPointer<vtkActor> gen_actor2(vtkSmartPointer<vtkPolyData>);
static vtkSmartPointer<vtkActor> gen_actor3(vtkSmartPointer<vtkPolyData>);
static vtkSmartPointer<vtkActor> gen_actor4(vtkSmartPointer<vtkPolyData>);

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
    vtkSmartPointer<vtkActor> resampleActor = gen_actor3(orig);
    vtkSmartPointer<vtkActor> sincActor = gen_actor4(orig);
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

    vtkSmartPointer<vtkRenderer> resampleRender =
        vtkSmartPointer<vtkRenderer>::New();
    resampleRender->SetViewport(rtView);
    resampleRender->AddActor(resampleActor);
    resampleRender->SetBackground(0, 1, 0);

    vtkSmartPointer<vtkRenderer> sincRender =
        vtkSmartPointer<vtkRenderer>::New();
    sincRender->SetViewport(rbView);
    sincRender->AddActor(sincActor);
    sincRender->SetBackground(0, 0.5, 0.5);
    //
    vtkSmartPointer<vtkRenderWindow> rw =
        vtkSmartPointer<vtkRenderWindow>::New();
    rw->AddRenderer(origRender);
    rw->AddRenderer(resampleRender);
    rw->AddRenderer(loopRender);
    rw->AddRenderer(sincRender);
    rw->SetSize(800, 600);
    rw->SetWindowName("PolyData Subdivision");

    origRender->GetActiveCamera()->SetPosition(0, -1, 0);
    origRender->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    origRender->GetActiveCamera()->SetViewUp(0, 0, 1);
    origRender->GetActiveCamera()->Azimuth(30);
    origRender->GetActiveCamera()->Elevation(30);
    origRender->ResetCamera();//刷新照相机
    //sync camera with origRender
    loopRender->SetActiveCamera(origRender->GetActiveCamera());
    resampleRender->SetActiveCamera(origRender->GetActiveCamera());
    sincRender->SetActiveCamera(origRender->GetActiveCamera());

    vtkSmartPointer<vtkRenderWindowInteractor> rwi =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    rwi->SetRenderWindow(rw);
    rwi->Start();
    rw->Render();

    return 0;
}
vtkSmartPointer<vtkActor> gen_actor4(vtkSmartPointer<vtkPolyData> _orig){
    h7::PerfHelper ph;
    ph.begin();
   // using FilterType = vtkSmartPointer<vtkWindowedSincPolyDataFilter>;
    using FilterType = vtkSmartPointer<vtkSmoothPolyDataFilter>;
    auto filter = FilterType::New();
    filter->SetInputData(_orig);
    //filter->SetFeatureEdgeSmoothing(true);
    //filter->SetBoundarySmoothing(true);
    filter->SetNumberOfIterations(5000);
    filter->Update();
    ph.print("gen_actor4");

    //add to actor for render
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(filter->GetOutput());
    vtkSmartPointer<vtkActor> loopActor =
        vtkSmartPointer<vtkActor>::New();
    loopActor->SetMapper(mapper);
    return loopActor;
}

//问题：多边形平滑后，图像大小改变（polyDataToImageData）。
//为了保持原来大小，vtk重采样后没有那么光滑了。
vtkSmartPointer<vtkActor> gen_actor3(vtkSmartPointer<vtkPolyData> _orig){
    vtkSmartPointer<vtkSmoothPolyDataFilter> filter =
            vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    filter->SetInputData(_orig);
    //开启后会变得尖锐。
    //filter->SetBoundarySmoothing(true);
    //filter->SetFeatureEdgeSmoothing(true);
    filter->SetNumberOfIterations(1000);
    filter->Update();
    //
    auto img_data = polyDataToImageData(filter->GetOutput());
    //resample. must be image data.
    vtkSmartPointer<vtkImageResample> reampler =
            vtkSmartPointer<vtkImageResample>::New();
    reampler->SetInputData(img_data);
    reampler->Update();

    int size[3];
    reampler->GetOutput()->GetDimensions(size);
    double spacing[3];
    reampler->GetOutput()->GetSpacing(spacing);

    reampler->SetOutputDimensionality(3);
    reampler->SetOutputSpacing(2, 2, 2);
    reampler->SetOutputOrigin(0, 0, 0);
    int dim[3] = {204, 147, 148}; //138, 69, 147
    reampler->SetOutputExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
    //重采样的方式
    reampler->SetInterpolationModeToNearestNeighbor();
    //reampler->SetInterpolationModeToCubic();
    //reampler->SetInterpolationModeToLinear();
    reampler->Update();

    //image data to poly data
    vtkNew<vtkDiscreteMarchingCubes> contour;
    contour->SetInputData(reampler->GetOutput());
    contour->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(contour->GetOutput());
    vtkSmartPointer<vtkActor> loopActor =
        vtkSmartPointer<vtkActor>::New();
    loopActor->SetMapper(mapper);
    return loopActor;
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

    //add to actor for render
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
