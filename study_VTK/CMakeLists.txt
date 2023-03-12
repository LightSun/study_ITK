cmake_minimum_required(VERSION 3.5)

project(study_VTK LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#SET(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/med)

SET(VTK_DEMO_NAME study_vtk)

add_executable(${VTK_DEMO_NAME}
    tests/main.cpp
    tests/VTKFlow.h
    tests/VTKFlow.cpp
    tests/VTKFlow_ctx.h
    tests/VTKFlow_pri.h
    tests/test_Cylinder.cpp
    tests/gen_random_number.cpp
    tests/test_PerspectiveTransform.cpp
    )
target_link_libraries(${VTK_DEMO_NAME} PUBLIC ${DEP_LIBS})
