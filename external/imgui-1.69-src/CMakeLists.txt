# Facesoft authored CMake build script for using ImGui

cmake_minimum_required (VERSION 3.5)
project(ImGui)

# build settings
set(CMAKE_CXX_STANDARD 11)
set(GLOBAL PROPERTY USE_FOLDERS ON )

# TODO: only include win32 implementation on windows
# TODO: include natvis on source files for windows only
# create imgui stub lib
file(GLOB IMGUI_SOURCES 
	LIST_DIRECTORIES true
	"*.cpp" "*.h")
add_library(ImGui ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC ${PROJECT_SOURCE_DIR})
