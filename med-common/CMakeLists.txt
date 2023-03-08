cmake_minimum_required(VERSION 3.5)

project(med-pro LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#SET(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/med)

SET(MED_LIB_NAME med-common)

add_library(${MED_LIB_NAME}
    med/MedFlow.hpp
    med/MedFlow.cpp
    med/MedFlow_ctx.h
    med/MedFlow_pri.h
    med/MedThyFilterManager.h
    med/MedThyFilterManager.cpp
    med/ImageStateManager.hpp
    med/vtk_impl.cpp
    )

target_link_libraries(${MED_LIB_NAME} PUBLIC ${DEP_LIBS})
if(NOT VTK_VERSION VERSION_LESS "8.90.0")
  vtk_module_autoinit(
    TARGETS ${MED_LIB_NAME}
    MODULES ${VTK_LIBRARIES}
  )
endif()

# test
SET(MED_TEST_NAME med_test)
add_executable(${MED_TEST_NAME}
    med/test/test.h
    med/test/test.cpp
    med/test/main.cpp
    )

target_link_libraries(${MED_TEST_NAME} PUBLIC ${MED_LIB_NAME})



