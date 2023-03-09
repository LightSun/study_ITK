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
static vtkSmartPointer<vtkImageData> polyDataToImageData2(
        vtkSmartPointer<vtkPolyData> polydata, double* spacing, int* dims);
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
    double _bounds[6];

    h7::PerfHelper ph;
    // Convert itkImage to vtkImage
    ph.begin();
    using ConvertFilter = itk::ImageToVTKImageFilter<ImageType>;
    ConvertFilter::Pointer itk_img2vtk = ConvertFilter::New();
    itk_img2vtk->SetInput(ptr);
    itk_img2vtk->Update();
    ph.print("img_to_vtkimg");

    //dims and spacing is ok
//    itk_img2vtk->GetOutput()->GetDimensions(_dims);
//    itk_img2vtk->GetOutput()->GetSpacing(_spacing);
//    printDims("img_to_vtkimg",_dims);
//    printSpacing("img_to_vtkimg", _spacing);

    //image data to poly data(坐标不对应了)
    vtkNew<vtkDiscreteMarchingCubes> contour;
    //contour->SetComputeAdjacentScalars(true);
    contour->SetInputData(itk_img2vtk->GetOutput());
    contour->Update();
    ph.print("img_to_poly");

    contour->GetOutput()->GetBounds(_bounds);
    printBounds("img_to_poly", _bounds);
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
            //polyDataToImageData2(filter->GetOutput(), spacing, dims);
    ph.print("poly_to_img");
    img_data->GetDimensions(_dims);
    printDims("poly_to_img",_dims);

//    img_data = vtk_resample(img_data, spacing, dims);
//    ph.print("vtk_resample");
//    img_data->GetDimensions(_dims);
//    printDims("vtk_resample",_dims);

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
//        med::ImageState s;
//        s.size = size;
//        s.origin = ret->GetOrigin();
//        s.spacing = ret->GetSpacing();
//        s.direction = ret->GetDirection();
//        ph.begin();
//        auto ptr_shadow = med::do_resample(ptr, &s);
//        ph.print("do_resample");
//        auto ret2 = med::do_global_registration(ptr_shadow, ret);
//        ph.print("do_global_registration");
//        return ret2;
    }
  //  img_to_poly bounds: (xmin,xmax, ymin,ymax, zmin,zmax) =
   //       (95.50,650.50,51.50,332.50,0.00,591.00)
   // poly bounds: (xmin,xmax, ymin,ymax, zmin,zmax) =
   //        (96.72,650.50,51.50,331.36,0.00,591.00)
//572, 287, 297
//484, 237, 297
}

vtkSmartPointer<vtkImageData> vtk_resample(vtkSmartPointer<vtkImageData> data,
                                           double* spacing, int* dim){
    vtkSmartPointer<vtkImageResample> reampler =
            vtkSmartPointer<vtkImageResample>::New();
    reampler->SetInputData(data);
    reampler->Update();
    int rawDims[3];
    reampler->GetOutput()->GetDimensions(rawDims);

    reampler->SetOutputDimensionality(3);
    reampler->SetOutputSpacing(spacing);
    reampler->SetOutputOrigin(0, 0, 0);
    //int dim[3] = {204, 147, 148}; //138, 69, 147
    reampler->SetOutputExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
    reampler->SetMagnificationFactors(dim[0] * 1.0 / rawDims[0],
            dim[1] * 1.0 / rawDims[1],
            dim[2] * 1.0 / rawDims[2]
            );
    //resample mode
    reampler->SetInterpolationModeToNearestNeighbor();
    //reampler->SetInterpolationModeToCubic();
    //reampler->SetInterpolationModeToLinear();
    reampler->Update();
    return reampler->GetOutput();
}

vtkSmartPointer<vtkImageData> polyDataToImageData(vtkSmartPointer<vtkPolyData>
                                                  polydata, double* spacing){
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    double bounds[6];
    polydata->GetBounds(bounds);
    LOGD("poly bounds: (xmin,xmax, ymin,ymax, zmin,zmax) = "
         "(%.2f,%.2f,%.2f,%.2f,%.2f,%.2f)\n",
         bounds[0], bounds[1], bounds[2],
         bounds[3], bounds[4], bounds[5]);
//    double spacing[3]; //desired volume spacing
//    spacing[0] = 1;
//    spacing[1] = 1;
//    spacing[2] = 1;
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
//wrong
vtkSmartPointer<vtkImageData> polyDataToImageData2(
        vtkSmartPointer<vtkPolyData> polydata, double* spacing, int* dims){
    double origin[3] = {0, 0, 0};
    int outval = 0;
    //
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->SetSpacing(spacing);
    imageData->SetDimensions(dims);
    imageData->SetExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, dims[2] - 1);
    imageData->SetOrigin(origin);
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

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
    imageStencil->ReverseStencilOff();
    imageStencil->SetBackgroundValue(outval);
    imageStencil->Update();

    imageData->DeepCopy(imageStencil->GetOutput());
    return imageData;
}
