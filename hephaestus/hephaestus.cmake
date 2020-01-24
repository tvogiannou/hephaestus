cmake_minimum_required (VERSION 3.5.1)


if(NOT DEFINED HEPHAESTUS_VULKAN_HEADERS_DIR)
    message(FATAL_ERROR "HEPHAESTUS_VULKAN_HEADERS_DIR needs be be defined")
endif()

# set sources
set(HEPHAESTUS_SOURCE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanDeviceManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanFunctionDispatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanGraphicsPipelineBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanHeadlessRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanMeshGraphicsPipeline.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanPrimitiveGraphicsPipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanRendererBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanValidate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VulkanSwapChainRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanDeviceManager.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanFunctionDispatcher.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanGraphicsPipelineBase.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanHeadlessRenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanMeshGraphicsPipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanPrimitiveGraphicsPipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanRendererBase.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanValidate.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanSwapChainRenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/VulkanPlatformConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/Platform.h
    ${CMAKE_CURRENT_LIST_DIR}/include/hephaestus/Log.h)
set(HEPHAESTUS_PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

# main target
add_library(hephaestus STATIC ${HEPHAESTUS_SOURCE_FILES})

target_include_directories(hephaestus PUBLIC "${HEPHAESTUS_PUBLIC_HEADERS_DIR}")
target_include_directories(hephaestus PUBLIC "${HEPHAESTUS_VULKAN_HEADERS_DIR}")
