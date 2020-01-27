cmake_minimum_required (VERSION 3.5.1)
project(demos)

option (HEPHAESTUS_PYTHON_BINDINGS "Generate target for python bindings" OFF)

set(HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../external")
set(CMAKE_CXX_STANDARD 11)

# build settings
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # set output directories for all projects to make it easier to track & test
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# compiler flags for the rest of the targets
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /GR- /WX /Zc:wchar_t-")
endif(MSVC)
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fno-rtti")

    if(HEPHAESTUS_PYTHON_BINDINGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -ldl -lpthread -fpic")   # rtti for pybind11
    endif()
endif(UNIX)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../hephaestus
                 ${CMAKE_CURRENT_BINARY_DIR}/hephaestus-build
                 EXCLUDE_FROM_ALL)

# utils shared by all demos
include(${CMAKE_CURRENT_LIST_DIR}/common/common.cmake)

# headless renderer demo
add_executable(headless-example ${CMAKE_CURRENT_LIST_DIR}/headless/RenderOBJToImageFile.cpp)
target_include_directories(headless-example PRIVATE "${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/stb")
target_link_libraries(headless-example PRIVATE hephaestus common ${CMAKE_DL_LIBS})

# real time app demo
include(${CMAKE_CURRENT_LIST_DIR}/app/app.cmake)

if(HEPHAESTUS_PYTHON_BINDINGS)
    add_subdirectory(${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/pybind11  ${CMAKE_CURRENT_BINARY_DIR}/pybind11-build)
    pybind11_add_module(hephaestus_bindings 
                        python/module.cpp
                        python/HephaestusBindingsUtils.cpp
                        python/module.h)
    target_link_libraries(hephaestus_bindings
            PRIVATE hephaestus common)
endif(HEPHAESTUS_PYTHON_BINDINGS)