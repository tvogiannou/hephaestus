cmake_minimum_required (VERSION 3.5.1)

if(NOT DEFINED HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION)
    message(FATAL_ERROR "HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION needs be be defined")
endif()

# add sources
set(APP_SOURCE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/imgui_impl_glfw.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_impl_vulkan.cpp
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/SimpleWindowGLFWImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WindowRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_impl_glfw.h
	${CMAKE_CURRENT_LIST_DIR}/imgui_impl_vulkan.h
	${CMAKE_CURRENT_LIST_DIR}/SimpleWindow.h
    ${CMAKE_CURRENT_LIST_DIR}/WindowRenderer.h)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(	"${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/glfw-3.3" 
                    ${CMAKE_CURRENT_BINARY_DIR}/glfw-build
                    EXCLUDE_FROM_ALL)

add_subdirectory("${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/imgui-1.69-src" ${CMAKE_CURRENT_BINARY_DIR}/imgui-build)

add_executable(app ${APP_SOURCE_FILES})
target_link_libraries(app PUBLIC hephaestus common glfw ImGui ${CMAKE_DL_LIBS})