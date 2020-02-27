#pragma once

#include <hephaestus/Platform.h>

// Utility header to forward to appropriate platform headers and config Vulkan preprocessor defines
// Should be included by files using Vulkan instead of including Vulkan headers directly

// platform specific configuration
#ifdef HEPHAESTUS_PLATFORM_WIN32
    #include <Windows.h>
    #define VK_USE_PLATFORM_WIN32_KHR

#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <dlfcn.h>
    #include <cstdlib>
    #define VK_USE_PLATFORM_XLIB_KHR

#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    #include <android/native_window.h>
    #include <dlfcn.h>
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

// forward to hephaestus assert
#define VULKAN_HPP_ASSERT HEPHAESTUS_ASSERT

#include <vulkan/vulkan.hpp>
