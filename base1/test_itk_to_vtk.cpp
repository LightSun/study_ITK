#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkNiftiImageIO.h>

#include <vtkActor.h>
#include <vtkAutoInit.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

#include "test_common.h"

void test_itk_to_vtk(int argc, char* argv[])
{
    // File name settings
    String fileName = "/media/heaven7/h7/3d_recreate/1-1_img.nii";
    if(argc > 1){
        fileName = argv[1];
    }

    // Define ImageType, ReaderType, ImageIOType and read file
    using PixelType = short;
    constexpr unsigned int Dimension = 3;
    using ImageType = itk::Image<PixelType, Dimension>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ImageIOType = itk::NiftiImageIO;
    ReaderType::Pointer reader = ReaderType::New();
    ImageIOType::Pointer niftiIO = ImageIOType::New();
    reader -> SetImageIO(niftiIO);
    reader -> SetFileName(fileName.data());
    reader -> Update();


    // Print img size
    ImageType::SizeType imgSize = reader -> GetOutput() -> GetLargestPossibleRegion().GetSize();
    std::cout << "read done! img size: " << imgSize << std::endl;


    // Convert itkImage to vtkImage
    using ConvertFilter = itk::ImageToVTKImageFilter<ImageType>;
    ConvertFilter::Pointer convertFilter = ConvertFilter::New();
    convertFilter -> SetInput(reader -> GetOutput());
    convertFilter -> Update();


    // Extract vtkImageData contour to vtkPolyData
    vtkNew<vtkDiscreteMarchingCubes> contour;
    contour -> SetInputData(convertFilter -> GetOutput());


    // Define colors, mapper, actor, renderer, renderWindow, renderWindowInteractor
    vtkNew<vtkNamedColors> colors;

    vtkNew<vtkPolyDataMapper> mapper;
    mapper -> SetInputConnection(contour -> GetOutputPort());

    vtkNew<vtkActor> actor;
    actor -> SetMapper(mapper);

    vtkNew<vtkRenderer> renderer;
    renderer -> AddActor(actor);
    renderer -> SetBackground(colors -> GetColor3d("SteelBlue").GetData());

    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow -> AddRenderer(renderer);

    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor -> SetRenderWindow(renderWindow);
    renderWindowInteractor -> Initialize();
    renderWindowInteractor -> Start();
     std::cout << "test_itk_to_vtk: end " << imgSize << std::endl;
}
//python
/*
import argparse
import itk
from vtkmodules.vtkCommonColor import vtkNamedColors
from vtkmodules.vtkFiltersGeneral import vtkDiscreteMarchingCubes
from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper, vtkRenderer, \
        vtkRenderWindow, vtkRenderWindowInteractor


def get_argument():
    parser = argparse.ArgumentParser()

    # Data settings
    parser.add_argument('--nifti_file_name', default='image.nii.gz')

    args = parser.parse_args()
    return args


def main():
    args = get_argument()

    # Read NIFTI file
    itk_img = itk.imread(filename=args.nifti_file_name)
    print(itk_img)

    # Convert itk to vtk
    vtk_img = itk.vtk_image_from_image(l_image=itk_img)
    print(vtk_img)

    # Extract vtkImageData contour to vtkPolyData
    contour = vtkDiscreteMarchingCubes()
    contour.SetInputData(vtk_img)

    # Define colors, mapper, actor, renderer, renderWindow, renderWindowInteractor
    colors = vtkNamedColors()

    mapper = vtkPolyDataMapper()
    mapper.SetInputConnection(contour.GetOutputPort())

    actor = vtkActor()
    actor.SetMapper(mapper)

    renderer = vtkRenderer()
    renderer.AddActor(actor)
    renderer.SetBackground(colors.GetColor3d("SteelBlue"))

    renderWindow = vtkRenderWindow()
    renderWindow.AddRenderer(renderer)

    renderWindowInteractor = vtkRenderWindowInteractor()
    renderWindowInteractor.SetRenderWindow(renderWindow)
    renderWindowInteractor.Initialize()
    renderWindowInteractor.Start()


if __name__ == '__main__':
    main()

 */
