#pragma once

#include <hephaestus/Compiler.h>

// Utility header to forward to appropriate platform headers and config Vulkan preprocessor defines
// Should be included by files using Vulkan instead of including Vulkan headers directly

// platform specific configuration
#ifdef HEPHAESTUS_PLATFORM_WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    #define VK_USE_PLATFORM_XLIB_KHR
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    #define VK_USE_PLATFORM_ANDROID_KHR
#else
    static_assert(false, "Unsupported platform");
#endif


// Vulkan configuration
// set the hephaestus dispatcher as the default one
namespace hephaestus 
{ 
    struct VulkanDispatcher; 
    const VulkanDispatcher& GetVulkanDispatcherInstance();
}
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_DEFAULT_DISPATCHER_TYPE ::hephaestus::VulkanDispatcher
#define VULKAN_HPP_DEFAULT_DISPATCHER hephaestus::GetVulkanDispatcherInstance()

// disable exceptions
#define VULKAN_HPP_NO_EXCEPTIONS

// forward to null assert to work around compiler warnings from the vulkan hpp header
#ifdef NDEBUG
#define HEPHAESTUS_NULL_ASSERT(expr) (void)(expr);
#define VULKAN_HPP_ASSERT HEPHAESTUS_NULL_ASSERT
#endif

// C function prototypes will be resolved by the hephaestus Vulkan dispatcher
#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.hpp>
