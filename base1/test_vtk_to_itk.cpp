#include <itkImage.h>

#include <itkVTKImageToImageFilter.h>

#include "vtkSmartPointer.h"
#include "vtkPNGReader.h"
#include <vtkImageLuminance.h>

#define ENABLE_QUICKVIEW

#ifdef ENABLE_QUICKVIEW
#  include "QuickView.h"
#endif

void
test_vtk_to_itk()
{
  //if (argc < 2)
   const char* file = "";
  vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
  reader->SetFileName(file);
  // reader->SetNumberOfScalarComponents(1); //doesn't seem to work - use ImageLuminance instead
  reader->Update();


  // Must convert image to grayscale because itkVTKImageToImageFilter only accepts single channel images
  vtkSmartPointer<vtkImageLuminance> luminanceFilter = vtkSmartPointer<vtkImageLuminance>::New();
  luminanceFilter->SetInputConnection(reader->GetOutputPort());
  luminanceFilter->Update();

  using ImageType = itk::Image<unsigned char, 2>;

  using VTKImageToImageType = itk::VTKImageToImageFilter<ImageType>;

  auto vtkImageToImageFilter = VTKImageToImageType::New();
  vtkImageToImageFilter->SetInput(luminanceFilter->GetOutput());
  // vtkImageToImageFilter->SetInput(reader->GetOutput());
  vtkImageToImageFilter->Update();

  auto image = ImageType::New();
  image->Graft(vtkImageToImageFilter->GetOutput()); // Need to do this because QuickView can't accept const

#ifdef ENABLE_QUICKVIEW
  QuickView viewer;
  viewer.AddImage(image.GetPointer()); // Need to do this because QuickView can't accept smart pointers
  viewer.Visualize();
#endif

}
