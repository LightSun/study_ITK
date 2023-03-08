#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkVTKImageToImageFilter.h>

#include "vtkDiscreteMarchingCubes.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPolyDataToImageStencil.h"
#include "vtkImageStencil.h"
#include "vtkPointData.h"

#include "MedThyFilterManager.h"
#include "common/common.h"

static vtkSmartPointer<vtkImageData> polyDataToImageData(
        vtkSmartPointer<vtkPolyData> polydata, double* spacing);

ImgPtr do_vtk_smooth(ImgPtr ptr, med::MedThy_Param* param){
    ImageType::SpacingType sts = ptr->GetSpacing();
    double spacing[MED_Dimension] = {sts[0], sts[1], sts[2]};
    MED_ASSERT(sts[0] != 0);

    h7::PerfHelper ph;
    // Convert itkImage to vtkImage
    ph.begin();
    using ConvertFilter = itk::ImageToVTKImageFilter<ImageType>;
    ConvertFilter::Pointer itk_img2vtk = ConvertFilter::New();
    itk_img2vtk->SetInput(ptr);
    itk_img2vtk->Update();
    ph.print("img_to_vtkimg");
    //image data to poly data
    vtkNew<vtkDiscreteMarchingCubes> contour;
    contour->SetInputData(itk_img2vtk->GetOutput());
    contour->Update();
    ph.print("img_to_poly");
    //smooth
    vtkSmartPointer<vtkSmoothPolyDataFilter> filter =
            vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    filter->SetInputData(contour->GetOutput());
    filter->SetNumberOfIterations(param->repeat_count);
    filter->Update();
    ph.print("vtk_smooth");

    //poly data to image data.
    auto img_data = polyDataToImageData(filter->GetOutput(), spacing);
    ph.print("poly_to_img");
    //image data to itk data.
    auto vtk_img2img = itk::VTKImageToImageFilter<ImageType>::New();
    vtk_img2img->SetInput(img_data);
    vtk_img2img->Update();
    ph.print("vtkimg_to_itkimg");
    return med::copyFilterOut(ptr, vtk_img2img->GetOutput()->GetBufferPointer());
}

vtkSmartPointer<vtkImageData> polyDataToImageData(vtkSmartPointer<vtkPolyData>
                                                  polydata, double* spacing){
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    double bounds[6];
    polydata->GetBounds(bounds);
//    double spacing[3]; //desired volume spacing
//    spacing[0] = 0.5;
//    spacing[1] = 0.5;
//    spacing[2] = 0.5;
    imageData->SetSpacing(spacing);

    // compute dimensions
    int dim[MED_Dimension];
    for (int i = 0; i < MED_Dimension; i++)
    {
        dim[i] = static_cast<int>(ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
    }
    imageData->SetDimensions(dim);
    imageData->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);

    double origin[MED_Dimension];
    for (int i = 0; i < MED_Dimension; i++){
        origin[i] = bounds[i * 2] + spacing[i] / 2;
    }
//    origin[0] = bounds[0] + spacing[0] / 2;
//    origin[1] = bounds[2] + spacing[1] / 2;
//    origin[2] = bounds[4] + spacing[2] / 2;
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

