cmake_minimum_required (VERSION 3.5.1)
project(hephaestus)

set(CMAKE_CXX_STANDARD 11)

# build settings
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # set output directories for all projects to make it easier to track & test
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


set(HEPHAESTUS_VULKAN_HEADERS_DIR "${CMAKE_CURRENT_LIST_DIR}/../external/vulkan-1.1.92")
include(${CMAKE_CURRENT_LIST_DIR}/hephaestus.cmake)

# compiler flags
if(MSVC)
    set_target_properties(hephaestus PROPERTIES COMPILE_FLAGS "/W4 /GR- /WX /Zc:wchar_t-")
endif(MSVC)
if(UNIX)
    set_target_properties(hephaestus PROPERTIES COMPILE_FLAGS "-Wall -Werror -fno-rtti")
endif(UNIX)
