cmake_minimum_required(VERSION 3.5)

project(study_ITK LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# VTK
#set(VTK_DIR /home/heaven7/heaven7/libs/ITK/VTK-9.2.6/build)
#find_package(VTK REQUIRED)

# ITK
#set(ITK_DIR /home/heaven7/heaven7/libs/ITK/build-InsightToolkit-5.2.1-Desktop_Qt_5_14_2_GCC_64bit-Release)
set(ITK_DIR /home/heaven7/heaven7/libs/ITK/build-InsightToolkit-5.3.0-Desktop_Qt_5_14_2_GCC_64bit-Release)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
if(VTK_VERSION VERSION_LESS "8.90.0")
  include(${VTK_USE_FILE})
endif()

set(EXE_NAME test1)

include_directories(${ITK_INCLUDE_DIRS})
include_directories(${ITKVtkGlue_INCLUDE_DIRS})
include_directories(${ITKVTK_INCLUDE_DIRS})

set(DEP_LIBS
    ${VTK_LIBRARIES}
    ${ITK_LIBRARIES}
    )
#ITK_LIBRARIES, ITK_LIBRARY_DIRS

add_executable(${EXE_NAME}
    base1/main.cpp
    base1/test_common.h
    base1/test_itk_to_vtk.cpp
    base1/test_vtk_to_itk.cpp
    base1/test_rgb_to_vtk.cpp
    base1/test_rgb_to_itk.cpp
    base1/test_regi_mean_square.cpp
    base1/test_regi_global_two_imgs.cpp
    base1/test_regi_feature_points.cpp
    base1/test_multi_resotion.cpp
    base1/test_mutualinformation.cpp
    base1/test_multi_model_mutual_Info.cpp
    base1/test_regi_landmarks.cpp
    base1/test_smoothing_binomial_blur.cpp
    base1/test_high_derivative.cpp
    base1/test_curvature_anisotropic.cpp
    base1/test_curvature_anisotropic_nifti.cpp
    base1/test_curvature_flow.cpp
    PerformanceHelper.h
    FileIO.h
    c_common.h
    nifti_reader.hpp
    base1/test_curvature_preserve_edge.cpp
    base1/test_regi_vector_imgs.cpp
    base1/print_nii.cpp
    base1/test_med.cpp
    base1/test_med2.cpp
    base1/test_med_regi.cpp
    NiftiStateManager.hpp
    )
target_link_libraries(${EXE_NAME} PUBLIC ${DEP_LIBS})

if(NOT VTK_VERSION VERSION_LESS "8.90.0")
  vtk_module_autoinit(
    TARGETS ${EXE_NAME}
    MODULES ${VTK_LIBRARIES}
  )
endif()
