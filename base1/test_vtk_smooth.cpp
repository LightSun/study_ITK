#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkTriangleFilter.h>

#include <vtkLoopSubdivisionFilter.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkButterflySubdivisionFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkXMLImageDataWriter.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkNiftiImageIO.h>
#include <itkTriangleMeshCurvatureCalculator.h>

#include "common/common.h"

//https://blog.csdn.net/shenziheng1/article/details/54848906
//网格平滑算法： https://www.cnblogs.com/chnhideyoshi/p/MeshSmoothing.html
int test_vtk_smooth(int argc, char* argv[])
{
    // File name settings
    String fileName = "/home/heaven7/heaven7/study/github/mine/"
                      "build-study_ITK-Desktop_Qt_5_14_2_GCC_64bit-Debug/"
                      "med-common/thy_resample4.nii";
    if(argc > 1){
        fileName = argv[1];
    }
    // Define ImageType, ReaderType, ImageIOType and read file
    using PixelType = unsigned char;
    constexpr unsigned int Dimension = 3;
    using ImageType = itk::Image<PixelType, Dimension>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ImageIOType = itk::NiftiImageIO;
    ReaderType::Pointer reader = ReaderType::New();
    ImageIOType::Pointer niftiIO = ImageIOType::New();
    reader-> SetImageIO(niftiIO);
    reader-> SetFileName(fileName.data());
    reader-> Update();

    // Print img size
    ImageType::SizeType imgSize = reader -> GetOutput() -> GetLargestPossibleRegion().GetSize();
    std::cout << "read done! img size: " << imgSize << std::endl;

    // Convert itkImage to vtkImage
    using ConvertFilter = itk::ImageToVTKImageFilter<ImageType>;
    ConvertFilter::Pointer convertFilter = ConvertFilter::New();
    convertFilter -> SetInput(reader -> GetOutput());
    convertFilter -> Update();
    std::cout << "ImageToVTKImageFilter done!" << std::endl;

    //save image
    //vtkImageWriter, vtkMetaImageWriter,vtkXMLImageDataWriter
//    vtkSmartPointer<vtkXMLImageDataWriter> meta_writer =
//            vtkSmartPointer<vtkXMLImageDataWriter>::New();
//    meta_writer->SetInputData(convertFilter->GetOutput());
//    meta_writer->SetFileName("thy_vtk.vti");
//    meta_writer->Update();
//    std::cout << "vtkXMLImageDataWriter done!" << std::endl;

    // Extract vtkImageData contour to vtkPolyData
    vtkNew<vtkDiscreteMarchingCubes> contour;
    contour->SetInputData(convertFilter -> GetOutput());
    contour->Update();
    std::cout << "vtkDiscreteMarchingCubes done!" << std::endl;

    vtkSmartPointer<vtkPolyDataWriter> vtk_writer =
            vtkSmartPointer<vtkPolyDataWriter>::New();
    vtk_writer->SetInputData(contour->GetOutput());
    vtk_writer->SetFileName("thy_vtk.vtk");
    vtk_writer->Update();
//    vtkSmartPointer<vtkPolyDataReader> reader =
//        vtkSmartPointer<vtkPolyDataReader>::New();
//    reader->SetFileName("fran_cut.vtk");
//    reader->Update();

    vtkSmartPointer<vtkSmoothPolyDataFilter> smply_filter =
        vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smply_filter->SetInputConnection(contour->GetOutputPort());
    smply_filter->SetNumberOfIterations(100);
    smply_filter->SetBoundarySmoothing(true);
    smply_filter->SetFeatureEdgeSmoothing(true);
    smply_filter->SetRelaxationFactor(1.0);
    smply_filter->Update();

    vtkSmartPointer<vtkPolyDataNormals> smoothFilter = vtkSmartPointer<vtkPolyDataNormals>::New();
    smoothFilter->SetInputConnection(smply_filter->GetOutputPort());
    smoothFilter->ComputePointNormalsOn();
    smoothFilter->ComputeCellNormalsOn();
    smoothFilter->Update();

    //多边形数据转化为三角形数据
//    vtkSmartPointer<vtkTriangleFilter> filter = vtkSmartPointer<vtkTriangleFilter>::New();
//    filter->SetInputData(contour->GetOutput());
//    filter->Update();
//    std::cout << "vtkTriangleFilter done!" << std::endl;

    //可以产生连续光滑的曲面，但是输入必须是三角形的
    //很耗内存。crash at 32G 内存 with 852597（点数） ，1674478(面数)
//    vtkSmartPointer<vtkLoopSubdivisionFilter> smoothFilter =
//        vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
//    smoothFilter->SetInputData(filter->GetOutput());
//    smoothFilter->SetNumberOfSubdivisions(4);
//    smoothFilter->Update();
//    std::cout << "vtkLoopSubdivisionFilter done!" << std::endl;

//    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoothFilter =
//        vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
//    smoothFilter->SetInputConnection(contour->GetOutputPort());
//    smoothFilter->SetNumberOfIterations(100);
//    //smoothFilter->SetBoundarySmoothing(true);
//   // smoothFilter->SetFeatureEdgeSmoothing(true);
//    smoothFilter->Update();
    //
    vtkSmartPointer<vtkPolyDataMapper> inputMapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    inputMapper->SetInputConnection(contour->GetOutputPort());
    vtkSmartPointer<vtkActor> inputActor =
        vtkSmartPointer<vtkActor>::New();
    inputActor->SetMapper(inputMapper);

    vtkSmartPointer<vtkPolyDataMapper> smoothedMapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    smoothedMapper->SetInputConnection(smoothFilter->GetOutputPort());
    vtkSmartPointer<vtkActor> smoothedActor =
        vtkSmartPointer<vtkActor>::New();
    smoothedActor->SetMapper(smoothedMapper);
    //
    double leftViewport[4] = { 0.0, 0.0, 0.5, 1.0 };
    double rightViewport[4] = { 0.5, 0.0, 1.0, 1.0 };

    vtkSmartPointer<vtkRenderer> leftRenderer =
        vtkSmartPointer<vtkRenderer>::New();
    leftRenderer->SetViewport(leftViewport);
    leftRenderer->AddActor(inputActor);
    leftRenderer->SetBackground(1, 0, 0);
    leftRenderer->ResetCamera();

    vtkSmartPointer<vtkRenderer> rightRenderer =
        vtkSmartPointer<vtkRenderer>::New();
    rightRenderer->SetViewport(rightViewport);
    rightRenderer->AddActor(smoothedActor);
    rightRenderer->SetBackground(0, 0, 0);
    rightRenderer->ResetCamera();
    //
    vtkSmartPointer<vtkRenderWindow> rw =
        vtkSmartPointer<vtkRenderWindow>::New();
    rw->AddRenderer(leftRenderer);
    rw->AddRenderer(rightRenderer);
    rw->SetSize(640, 320);
    rw->SetWindowName("PolyData Grid Smooth By LapLasian");

    vtkSmartPointer<vtkRenderWindowInteractor> rwi =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    rwi->SetRenderWindow(rw);
    rwi->Initialize();
    rwi->Start();
    return 0;
}
