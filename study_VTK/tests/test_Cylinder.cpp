
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkMath.h>

#include "VTKFlow.h"

int test_Cylinder(int argc, char* argv[]){

     //==== distance of two points
     double p0[3] = {0.0, 0.0, 0.0};
     double p1[3] = {1.0, 1.0, 1.0};

     // Find the squared distance between the points.
     double squaredDistance = vtkMath::Distance2BetweenPoints(p0, p1);

     // Take the square root to get the Euclidean distance between the points.
     double distance = std::sqrt(squaredDistance);
     printf("distance = %.3f\n", distance);
     //=====    vtkCylinderSource
     vtkNew<vtkNamedColors> colors;

     // Set the background color.
     std::array<unsigned char, 4> bkg{{26, 51, 102, 255}};
     colors->SetColor("BkgColor", bkg.data());

     // This creates a polygonal cylinder model with eight circumferential facets
     // (i.e, in practice an octagonal prism).
     vtkNew<vtkCylinderSource> cylinder;
     cylinder->SetResolution(8);

     // The mapper is responsible for pushing the geometry into the graphics
     // library. It may also do color mapping, if scalars or other attributes are
     // defined.
     vtkNew<vtkPolyDataMapper> cylinderMapper;
     cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

     // The actor is a grouping mechanism: besides the geometry (mapper), it
     // also has a property, transformation matrix, and/or texture map.
     // Here we set its color and rotate it around the X and Y axes.
     vtkNew<vtkActor> cylinderActor;
     cylinderActor->SetMapper(cylinderMapper);
     cylinderActor->GetProperty()->SetColor(
         colors->GetColor4d("Tomato").GetData());
     cylinderActor->RotateX(30.0);
     cylinderActor->RotateY(-45.0);

     h7::VTKFlow flow;
     flow.show(cylinderActor.GetPointer());

     return 0;
}
