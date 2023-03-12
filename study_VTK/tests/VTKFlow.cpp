#include "VTKFlow.h"
#include "VTKFlow_pri.h"

#include <vtkRendererCollection.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkProperty.h>

#include <math.h>

static inline void min_square(unsigned int src, int out[2]){
    unsigned int val, dst;
    val = (unsigned int)sqrt(src);
    while (1) {
        dst = val * val;
        if(dst >= src){
            break;
        }
        val ++;
    }
    if(val - 1 > 0 && val * (val - 1) >= src){
        out[0] = val;
        out[1] = val - 1;
    }else{
        out[0] = val;
        out[1] = val;
    }
}

namespace h7 {

vtkSmartPointer<vtkActor> VTKFlow::newActor(vtkSmartPointer<vtkPolyData> data, CString color){
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(data);
    mapper->Update();
    // std::array<unsigned char, 4> bkg{{26, 51, 102, 255}};
    // colors->SetColor("BkgColor", bkg.data());
    vtkNew<vtkNamedColors> colors;
    // The actor is a grouping mechanism: besides the geometry (mapper), it
    // also has a property, transformation matrix, and/or texture map.
    // Here we set its color and rotate it around the X and Y axes.
    vtkSmartPointer<vtkActor> cylinderActor = vtkSmartPointer<vtkActor>::New();
    cylinderActor->SetMapper(mapper);
    cylinderActor->GetProperty()->SetColor(
        colors->GetColor4d(color).GetData());
    //cylinderActor->RotateX(30.0);
    //cylinderActor->RotateY(-45.0);
    return cylinderActor;
}

void VTKFlow::show(vtkSmartPointer<vtkActor> actor){
    vtkNew<vtkNamedColors> colors;

     // Set the background color.
     std::array<unsigned char, 4> bkg{{26, 51, 102, 255}};
     colors->SetColor("BkgColor", bkg.data());
    // The renderer generates the image
    // which is then displayed on the render window.
    // It can be thought of as a scene to which the actor is added
    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
    // Zoom in a little by accessing the camera and invoking its "Zoom" method.
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Zoom(1.5);

    // The render window is the actual GUI window
    // that appears on the computer screen
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->SetSize(300, 300);
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("Cylinder");

    // The render window interactor captures mouse events
    // and will perform appropriate camera or actor manipulation
    // depending on the nature of the events.
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // This starts the event loop and as a side effect causes an initial render.
    renderWindow->Render();
    renderWindowInteractor->Start();
}

void VTKFlow::show(const std::vector<vtkSmartPointer<vtkActor>> &actors){
    MED_ASSERT(actors.size() > 1);
    int act_size = (int)actors.size();
    int wh[2];
    min_square(act_size, wh);
    //
    double color_delta = 1.0 / act_size;
    //
    double x_every = 1.0/ wh[0];
    double y_every = 1.0/ wh[1];
    double viewPort[4];
    double bg_color[4];
    double x_pos, y_pos;
    vtkSmartPointer<vtkRenderWindow> rw =
        vtkSmartPointer<vtkRenderWindow>::New();
    for(int i = 0 ; i < act_size ; ++i){
        //(xmin,ymin,xmax,ymax), 0-1
        y_pos = (i + 1) * 1.0/ wh[0];
        x_pos = (i + 1) % wh[0];
        //12 --- 4*4 --- i = 5 -> x_pos = 2, y_pos = 1.
        viewPort[0] = x_every * (x_pos - 1);
        viewPort[2] = x_every * (x_pos);
        viewPort[1] = y_every * (y_pos);
        viewPort[3] = y_every * (y_pos + 1);

        gen_bg_color(i*color_delta, bg_color);
        vtkSmartPointer<vtkRenderer> render =
            vtkSmartPointer<vtkRenderer>::New();
        render->SetViewport(viewPort);
        render->AddActor(actors[i]);
        render->SetBackground(bg_color[0], bg_color[1],bg_color[2]);
        render->SetBackgroundAlpha(bg_color[3]);
        rw->AddRenderer(render);
    }
    rw->SetSize(m_params.window_width, m_params.window_height);
    rw->SetWindowName(m_params.window_name.data());
    //
    auto origRender = rw->GetRenderers()->GetFirstRenderer();

    origRender->GetActiveCamera()->SetPosition(0, -1, 0);
    origRender->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    origRender->GetActiveCamera()->SetViewUp(0, 0, 1);
    origRender->GetActiveCamera()->Azimuth(30);
    origRender->GetActiveCamera()->Elevation(30);
    origRender->ResetCamera();
    //sync camera with origRender
    while (true) {
        auto item = rw->GetRenderers()->GetNextItem();
        if(item == nullptr){
            break;
        }
        item->SetActiveCamera(origRender->GetActiveCamera());
    }
    vtkSmartPointer<vtkRenderWindowInteractor> rwi =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    rwi->SetRenderWindow(rw);
    rwi->Start();
    rw->Render();
}

void VTKFlow::gen_bg_color(double delta, double* out){
    switch (m_params.bg_change_mode) {
    case kBackground_change_R:{
        out[0] = delta;
        out[1] = 0;
        out[2] = 0;
        out[3] = 0;
    }break;
    case kBackground_change_G:{
        out[0] = 0;
        out[1] = delta;
        out[2] = 0;
        out[3] = 0;
    }break;
    case kBackground_change_B:{
        out[0] = 0;
        out[1] = 0;
        out[2] = delta;
        out[3] = 0;
    }break;
    case kBackground_change_A:{
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        out[3] = delta;
    }break;
    case kBackground_change_RGBA:{
        out[0] = delta;
        out[1] = delta;
        out[2] = delta;
        out[3] = delta;
    }break;

    default:
        fprintf(stderr, "wrong chang mode(%d).\n", m_params.bg_change_mode);
        abort();
    }
}

}
