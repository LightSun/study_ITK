#ifndef CTX_PRI_H
#define CTX_PRI_H

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

#include "common/common.h"

#endif // CTX_PRI_H
