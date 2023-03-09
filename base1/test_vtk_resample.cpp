
#include "vtkMetaImageReader.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkPolyDataReader.h"
#include "vtkImageResample.h"
#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkPolyData.h>
#include "vtkMetaImageReader.h"
#include "vtkImageReader.h"
#include "vtkMetaImageWriter.h"
#include "vtkImageWriter.h"

#include "vtkImageStencil.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkDiscreteMarchingCubes.h"
#include "vtkPolyDataToImageStencil.h"
#include "vtkImageToPolyDataFilter.h"

#define VTK_SP vtkSmartPointer

vtkSmartPointer<vtkImageData> polyDataToImageData(vtkSmartPointer<vtkPolyData>);
static vtkSmartPointer<vtkImageData> polyDataToImageData2(vtkSmartPointer<vtkPolyData>);

int test_vtk_resample(int argc, char* argv[]){
    vtkNew<vtkImageDataGeometryFilter> imageDataGeometryFilter;

    vtkSmartPointer<vtkPolyDataReader> reader =
            vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName("thy_vtk.vtk");
    reader->Update();
    std::cout << "read done: " << std::endl;

    vtkSmartPointer<vtkImageData> img_data = polyDataToImageData(reader->GetOutput());
//    int dim[3];
//    reader->GetOutput()->GetDimensions(dim);
//    double origin[3];
//    reader->GetOutput()->GetOrigin(origin);
//    double spaceing[3];
//    reader->GetOutput()->GetSpacing(spaceing);
//    auto dim_str = std::to_string(dim[0]) + "_" + std::to_string(dim[1])
//            + "_" + std::to_string(dim[2]);
//    std::cout << "dim_str: " << dim_str << std::endl;

    vtkSmartPointer<vtkImageResample> reampler =
            vtkSmartPointer<vtkImageResample>::New();
    //reampler->SetInputConnection(reader->GetOutputPort());
    reampler->SetInputData(img_data);
    reampler->Update();

    reampler->SetOutputDimensionality(3);
    reampler->SetOutputSpacing(2, 2, 2);
    //x,y,z各自减少 一半
    reampler->SetAxisMagnificationFactor(0, 0.3);
    reampler->SetAxisMagnificationFactor(1, 0.3);
    reampler->SetAxisMagnificationFactor(2, 0.3);
    //reampler->SetInterpolationModeToLinear();
    //reampler->SetOutputSpacing(spaceing2);
    reampler->SetInterpolationModeToNearestNeighbor();//重采样的方式
    reampler->Update();
    std::cout << "reampler done." << std::endl;

    if (reampler->GetOutput()){
        //image data to poly data
        vtkNew<vtkDiscreteMarchingCubes> contour;
        contour->SetInputData(reampler -> GetOutput());
        contour->Update();

        vtkSmartPointer<vtkPolyDataWriter> writer =
                vtkSmartPointer<vtkPolyDataWriter>::New();
        writer->SetInputData(contour->GetOutput());
        writer->SetFileName("thy_vtk_resample.vtk");
        writer->Write();
        std::cout << "thy_vtk_resample write done." << std::endl;
    }
    return 0;
}

static vtkSmartPointer<vtkImageData> polyDataToImageData2(
        vtkSmartPointer<vtkPolyData> pd){
    // Convert the image to a polydata
    vtkNew<vtkImageDataGeometryFilter> imageDataGeometryFilter;
    imageDataGeometryFilter->SetInputData(pd);
    imageDataGeometryFilter->Update();

    vtkNew<vtkImageToPolyDataFilter> filter;

    filter->SetInputData(pd);
    filter->SetOutputStyleToPolygonalize();
    filter->SetError(0);
    filter->DecimationOn();
    filter->SetDecimationError(0.0);
    filter->SetSubImageSize(25);
    //not done
}

vtkSmartPointer<vtkImageData> polyDataToImageData(vtkSmartPointer<vtkPolyData> polydata)
{
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    double bounds[6];
    polydata->GetBounds(bounds);
    double spacing[3]; //desired volume spacing
    spacing[0] = 0.5;
    spacing[1] = 0.5;
    spacing[2] = 0.5;
    imageData->SetSpacing(spacing);

    // compute dimensions
    int dim[3];
    for (int i = 0; i < 3; i++)
    {
        dim[i] = static_cast<int>(ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
    }
    imageData->SetDimensions(dim);
    //SetExtent 设置每个轴的尺寸。
    imageData->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);

    double origin[3];
    origin[0] = bounds[0] + spacing[0] / 2;
    origin[1] = bounds[2] + spacing[1] / 2;
    origin[2] = bounds[4] + spacing[2] / 2;
    imageData->SetOrigin(origin);

    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    // fill the image with foreground voxels:
    unsigned char inval = 255;
    unsigned char outval = 0;
    vtkIdType count = imageData->GetNumberOfPoints();
    for (vtkIdType i = 0; i < count; ++i)
    {
        imageData->GetPointData()->GetScalars()->SetTuple1(i, inval);
    }

    //polygonal data --> image stencil:
    vtkSmartPointer<vtkPolyDataToImageStencil> pdtoImageStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pdtoImageStencil->SetInputData(polydata);
    pdtoImageStencil->SetOutputOrigin(origin);
    pdtoImageStencil->SetOutputSpacing(spacing);
    pdtoImageStencil->SetOutputWholeExtent(imageData->GetExtent());
    pdtoImageStencil->Update();

    //cut the corresponding white image and set the background:
    vtkSmartPointer<vtkImageStencil> imageStencil = vtkSmartPointer<vtkImageStencil>::New();
    imageStencil->SetInputData(imageData);
    imageStencil->SetStencilConnection(pdtoImageStencil->GetOutputPort());
    imageStencil->ReverseStencilOff();
    imageStencil->SetBackgroundValue(outval);
    imageStencil->Update();

    imageData->DeepCopy(imageStencil->GetOutput());
    return imageData;
}
