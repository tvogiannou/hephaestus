#pragma once

#include <hephaestus/Platform.h>

// Utility header to forward to appropiate platform
// Should be included by files using Vulkan instead of including Vulkan headers directly

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

#include <vulkan/vulkan.hpp>
