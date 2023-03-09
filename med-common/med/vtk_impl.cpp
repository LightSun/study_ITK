#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkVTKImageToImageFilter.h>
#include <itkCastImageFilter.h>

#include "vtkDiscreteMarchingCubes.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPolyDataToImageStencil.h"
#include "vtkImageStencil.h"
#include "vtkPointData.h"
#include "vtkImageResample.h"
#include "vtkImageToPolyDataFilter.h"

#include "MedThyFilterManager.h"
#include "common/common.h"

static inline void printDims(CString tag, int* dim){
    LOGD("%s >> dims = %d, %d, %d\n", tag.data(), dim[0], dim[1], dim[2]);
}
static inline void printSpacing(CString tag, double* dim){
    LOGD("%s >> spacing = %.3f, %.3f, %.3f\n", tag.data(), dim[0], dim[1], dim[2]);
}
static inline void printBounds(CString tag, double* bounds){
    LOGD("%s bounds: (xmin,xmax, ymin,ymax, zmin,zmax) = "
         "(%.2f,%.2f,%.2f,%.2f,%.2f,%.2f)\n",tag.data(),
         bounds[0], bounds[1], bounds[2],
         bounds[3], bounds[4], bounds[5]);
}

static vtkSmartPointer<vtkImageData> polyDataToImageData(
        vtkSmartPointer<vtkPolyData> polydata, double* spacing);
static vtkSmartPointer<vtkImageData> vtk_resample(
        vtkSmartPointer<vtkImageData> data, double* spacing, int* sizes);

ImgPtr do_vtk_smooth(ImgPtr ptr, med::MedThy_Param* param){
    double spacing[MED_Dimension];
    int dims[MED_Dimension];
    {
        ImageType::SpacingType sts = ptr->GetSpacing();
        MED_ASSERT(sts[0] != 0);
        for(int i = 0 ; i < MED_Dimension ; ++i){
            spacing[i] = sts[i];
        }
        ImageType::SizeType sizes = ptr->GetLargestPossibleRegion().GetSize();
        for(int i = 0 ; i < MED_Dimension ; ++i){
            dims[i] = sizes.GetElement(i);
        }
    }
    //
    printDims("pre_do_vtk_smooth", dims);
    int _dims[3];
    double _spacing[3];
   // double _bounds[6];

    h7::PerfHelper ph;
    // Convert itkImage to vtkImage
    ph.begin();
    using ConvertFilter = itk::ImageToVTKImageFilter<ImageType>;
    ConvertFilter::Pointer itk_img2vtk = ConvertFilter::New();
    itk_img2vtk->SetInput(ptr);
    itk_img2vtk->Update();
    ph.print("img_to_vtkimg");

    //image data to poly data(坐标不对应了)
    vtkNew<vtkDiscreteMarchingCubes> contour;
    //contour->SetComputeAdjacentScalars(true);
    contour->SetInputData(itk_img2vtk->GetOutput());
    contour->Update();
    ph.print("img_to_poly");

    //contour->GetOutput()->GetBounds(_bounds);
    //printBounds("img_to_poly", _bounds);
    //smooth
    vtkSmartPointer<vtkSmoothPolyDataFilter> filter =
            vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    filter->SetInputData(contour->GetOutput());
    filter->SetNumberOfIterations(param->repeat_count);
    filter->Update();
    ph.print("vtk_smooth");

    //poly data to image data.
    vtkSmartPointer<vtkImageData> img_data =
            polyDataToImageData(filter->GetOutput(), spacing);
    ph.print("poly_to_img");
    img_data->GetDimensions(_dims);
    printDims("poly_to_img",_dims);

    //image data to itk data.
    auto vtk_img2img = itk::VTKImageToImageFilter<ImageType_I8>::New();
    vtk_img2img->SetInput(img_data);
    vtk_img2img->Update();
    ph.print("vtkimg_to_itkimg");

    auto cast = itk::CastImageFilter<ImageType_I8, ImageType>::New();
    cast->SetInput(vtk_img2img->GetOutput());
    cast->Update();
    //size changed.
    {
        auto out = cast->GetOutput();
        auto size = out->GetLargestPossibleRegion().GetSize();
#if 0
        {
            int sizes[3];
            sizes[0] = size.GetElement(0);
            sizes[1] = size.GetElement(1);
            sizes[2] = size.GetElement(2);
            printDims("cast_ret", sizes);
            //1,1,1
            auto _ss = out->GetSpacing();
            _spacing[0] = _ss[0];
            _spacing[1] = _ss[1];
            _spacing[2] = _ss[2];
            printSpacing("cast_ret", _spacing);
        }
#endif
        auto ret = ImageType::New();
        ret->SetSpacing(out->GetSpacing());
        ret->SetRegions(size);
        //need this
        ret->SetOrigin(img_data->GetOrigin());
        ret->Allocate();

        memcpy(ret->GetBufferPointer(), out->GetBufferPointer(),
               size.GetElement(0) * size.GetElement(1) * size.GetElement(2)
                * sizeof (ImageType::PixelType)
               );
        return ret;
        //resample and registration? wrong for med
    }
}

vtkSmartPointer<vtkImageData> polyDataToImageData(vtkSmartPointer<vtkPolyData>
                                                  polydata, double* _spacing){
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    double bounds[6];
    polydata->GetBounds(bounds);
    // desired volume spacing
#define PLOY_SPACING 1
    double spacing[3] = {PLOY_SPACING, PLOY_SPACING, PLOY_SPACING};
#undef PLOY_SPACING
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
    vtkSmartPointer<vtkPolyDataToImageStencil> pdtoImageStencil =
            vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pdtoImageStencil->SetInputData(polydata);
    pdtoImageStencil->SetOutputOrigin(origin);
    pdtoImageStencil->SetOutputSpacing(spacing);
    pdtoImageStencil->SetOutputWholeExtent(imageData->GetExtent());
    pdtoImageStencil->Update();

    //cut the corresponding white image and set the background:
    vtkSmartPointer<vtkImageStencil> imageStencil =
            vtkSmartPointer<vtkImageStencil>::New();
    imageStencil->SetInputData(imageData);
    imageStencil->SetStencilConnection(pdtoImageStencil->GetOutputPort());
    //on: 设置开启翻转像素，则模具外的数据就会填充为1，模具内的数据填充为0；
    imageStencil->ReverseStencilOff();
    imageStencil->SetBackgroundValue(outval);
    imageStencil->Update();

    imageData->DeepCopy(imageStencil->GetOutput());
    return imageData;
}
