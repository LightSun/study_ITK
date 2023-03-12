cmake_minimum_required(VERSION 3.5)

project(study_ITK LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Cmake command line too long
if(WIN32)
    SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)

    SET(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
    SET(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")
    #ninja
    SET(CMAKE_NINJA_FORCE_RESPONSE_FILE 1 CACHE INTERNAL "")
endif(WIN32)

# VTK
#set(VTK_DIR /home/heaven7/heaven7/libs/ITK/VTK-9.2.6/build)
#find_package(VTK REQUIRED)

# ITK
set(ITK_DIR)
if(WIN32)
    set(ITK_DIR D:/study/cpp/ITK/InsightToolkit-5.3.0/build)
    #SET(VTK_DIR D:/study/cpp/ITK/VTK-9.2.6/build)
elseif(UNIX)
    #set(ITK_DIR /home/heaven7/heaven7/libs/ITK/build-InsightToolkit-5.2.1-Desktop_Qt_5_14_2_GCC_64bit-Release)
    set(ITK_DIR /home/heaven7/heaven7/libs/ITK/build-InsightToolkit-5.3.0-Desktop_Qt_5_14_2_GCC_64bit-Release)
    #set(ITK_DIR /home/heaven7/heaven7/libs/ITK/InsightToolkit-5.3.0/build)
else()
endif()

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
#include(${ITK_DIR}/ITKConfig.cmake)
if(VTK_VERSION VERSION_LESS "8.90.0")
  include(${VTK_USE_FILE})
endif()

include_directories(${ITK_INCLUDE_DIRS})
include_directories(${ITKVtkGlue_INCLUDE_DIRS})
include_directories(${ITKVTK_INCLUDE_DIRS})

set(DEP_LIBS
    ${VTK_LIBRARIES}
    ${ITK_LIBRARIES}
    )

# h7cpp
SET(H7CPP_DIR /home/heaven7/heaven7/env/linux/h7cpp)
SET(H7CPP_LIB ${H7CPP_DIR}/lib/libh7cpp_shared.so)
if(WIN32)
   SET(H7CPP_DIR D:/study/cpp/ENV/h7cpp)
   SET(H7CPP_LIB ${H7CPP_DIR}/lib/libh7cpp_shared.dll.a)
endif(WIN32)
SET(H7CPP_INC ${H7CPP_DIR}/include)

include_directories(${H7CPP_INC})
set(DEP_LIBS ${DEP_LIBS}
    ${H7CPP_LIB}
    )

#
add_subdirectory(med-common/)
add_subdirectory(mesh-smoothing/)
add_subdirectory(study_VTK/)

#SET(MED_DIR med-common)
#add_library(med-common
#    ${MED_DIR}/med/MedFlow.hpp
#    ${MED_DIR}/med/MedFlow.cpp
#    ${MED_DIR}/med/main.cpp
#    ${MED_DIR}/med/MedFlow_ctx.h
#    ${MED_DIR}/med/MedFlow_pri.h
#    ${MED_DIR}/med/MedThyFilterManager.h
#    ${MED_DIR}/med/MedThyFilterManager.cpp
#    )
#target_link_libraries(med-common PUBLIC ${DEP_LIBS})

# test
set(EXE_NAME test1)

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
    base1/test_curvature_preserve_edge.cpp
    base1/test_regi_vector_imgs.cpp
    base1/print_nii.cpp
    base1/test_med.cpp
    base1/test_med2.cpp
    base1/test_med_regi.cpp
    nifti_reader.hpp
    NiftiStateManager.hpp

    base1/test_vtk_smooth.cpp
    base1/test_vtk_smooth2.cpp
    base1/test_vtk_resample.cpp
   # base1/test_rtk_entry.cpp
    )
target_link_libraries(${EXE_NAME} PUBLIC ${DEP_LIBS})

if(NOT VTK_VERSION VERSION_LESS "8.90.0")
  vtk_module_autoinit(
    TARGETS ${EXE_NAME}
    MODULES ${VTK_LIBRARIES}
  )
endif()
