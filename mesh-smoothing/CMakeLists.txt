cmake_minimum_required(VERSION 3.5)

project(med-pro LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#SET(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/med)

SET(MESH_LIB_NAME mesh-smoothing)

add_library(${MESH_LIB_NAME}
    triangle/Triangle.h
    triangle/Mesh.h
    triangle/Laplacian.hpp
    triangle/HCLaplacian.hpp
    triangle/CurvatureFlow.hpp
    triangle/Taubin.hpp
    triangle/Smoothing.h
    triangle/Smoothing.cpp
    triangle/main.cpp
    )
