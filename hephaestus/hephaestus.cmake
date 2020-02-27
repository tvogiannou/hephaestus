cmake_minimum_required (VERSION 3.5.1)

if(NOT DEFINED HEPHAESTUS_VULKAN_HEADERS_DIR)
    message(FATAL_ERROR "HEPHAESTUS_VULKAN_HEADERS_DIR needs be be defined")
endif()

# set sources
set(HEPHAESTUS_SOURCE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanDeviceManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanDispatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/PipelineBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/HeadlessRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/TriMeshPipeline.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/src/PrimitivesPipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/RendererBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanValidate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/SwapChainRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanDeviceManager.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanDispatcher.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/PipelineBase.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/HeadlessRenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/TriMeshPipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/PrimitivesPipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/RendererBase.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanValidate.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/SwapChainRenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/Platform.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/Log.h)
set(HEPHAESTUS_PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

# main target
add_library(hephaestus STATIC ${HEPHAESTUS_SOURCE_FILES})

target_include_directories(hephaestus PUBLIC "${HEPHAESTUS_PUBLIC_HEADERS_DIR}")
target_include_directories(hephaestus PUBLIC "${HEPHAESTUS_VULKAN_HEADERS_DIR}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_link_libraries(hephaestus PUBLIC log)
endif()
