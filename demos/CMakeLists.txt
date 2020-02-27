cmake_minimum_required (VERSION 3.5.1)
project(demos)

option (HEPHAESTUS_PREVIEWER_APP "Generate target for previewr app" ON)
option (HEPHAESTUS_HEADLESS_EXAMPLE "Generate target for headless example" ON)
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

# add the hephaestus library
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../hephaestus
                 ${CMAKE_CURRENT_BINARY_DIR}/hephaestus-build
                 EXCLUDE_FROM_ALL)

# add common utils shared by all demos
include(${CMAKE_CURRENT_LIST_DIR}/common/common.cmake)

# add headless renderer demo
if (HEPHAESTUS_HEADLESS_EXAMPLE)
include(${CMAKE_CURRENT_LIST_DIR}/headless/headless.cmake)
endif()

# non Android targets
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
    # real time app demo
    if(HEPHAESTUS_PREVIEWER_APP)
    include(${CMAKE_CURRENT_LIST_DIR}/previewer-app/previewer-app.cmake)
    endif()
    
    # python bindings
    if(HEPHAESTUS_PYTHON_BINDINGS)
    set(HEPHAESTUS_PYBIND11_HEADERS_DIR "${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/pybind11")
    include(${CMAKE_CURRENT_LIST_DIR}/python/hephaestus-bindings.cmake)
    endif(HEPHAESTUS_PYTHON_BINDINGS)
endif()
