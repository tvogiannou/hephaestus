#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanConfig.h>


namespace hephaestus
{

// Dispatcher to be used with Vulkan.hpp objects so that Vulkan functions can be loaded dynamically
// See DispatchLoaderDynamic in vulkan.hpp for available functions (per platform)
struct VulkanDispatcher
{

#ifdef HEPHAESTUS_PLATFORM_WIN32
    typedef HMODULE ModuleType; // win32
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    typedef void* ModuleType;
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    typedef void* ModuleType;
#endif

    static VulkanDispatcher& GetInstance();

    static void InitFromLibrary(ModuleType vulkanLibrary);
    static void LoadGlobalFunctions();
    static void LoadInstanceFunctions(const vk::Instance& instance);
    static void LoadDeviceFunctions(const vk::Device& device);

    // dynamic loader function for loading the rest of the Vulkan commands
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

    // declare all functions that we need to load for the dispatcher
#define VULKAN_EXPORTEDFUNCTION_DECLARATION(fun) PFN_##fun fun = 0
#include "VulkanFunctions.inl"
#undef VULKAN_EXPORTEDFUNCTION_DECLARATION

};

} // hephaestus