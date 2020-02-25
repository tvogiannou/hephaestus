
#include <hephaestus/VulkanDispatcher.h>

#include <hephaestus/Log.h>



#define HEPHAESTUS_VK_DISPATCHER_LOAD_GLOBAL_FUNCTION(fun) VulkanDispatcher::GetInstance().fun = \
    (PFN_##fun)VulkanDispatcher::GetInstance().vkGetInstanceProcAddr(nullptr, #fun)
#define HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(fun, object) VulkanDispatcher::GetInstance().fun = \
    (PFN_##fun)object.getProcAddr(#fun, VulkanDispatcher::GetInstance())

namespace hephaestus
{

hephaestus::VulkanDispatcher s_dispatcherInstance;

VulkanDispatcher& VulkanDispatcher::GetInstance()
{
    return s_dispatcherInstance;
}

void 
VulkanDispatcher::InitFromLibrary(ModuleType vulkanLibrary)
{
    HEPHAESTUS_LOG_ASSERT(s_dispatcherInstance.vkGetInstanceProcAddr == 0, "vkGetInstanceProcAddr should not be set");

#ifdef HEPHAESTUS_PLATFORM_WIN32
    s_dispatcherInstance.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkanLibrary, "vkGetInstanceProcAddr");
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    s_dispatcherInstance.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vulkanLibrary, "vkGetInstanceProcAddr");
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    s_dispatcherInstance.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vulkanLibrary, "vkGetInstanceProcAddr");
#else
    static_assert(false, "VulkanFunctionDispatcher: Unknown platform configuration");
#endif

    HEPHAESTUS_LOG_ASSERT(s_dispatcherInstance.vkGetInstanceProcAddr, "Failed to get vkGetInstanceProcAddr");
}

void 
VulkanDispatcher::LoadGlobalFunctions()
{
    HEPHAESTUS_LOG_ASSERT(s_dispatcherInstance.vkGetInstanceProcAddr, "Dispatcher has not been initialized");

    HEPHAESTUS_VK_DISPATCHER_LOAD_GLOBAL_FUNCTION(vkCreateInstance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_GLOBAL_FUNCTION(vkEnumerateInstanceVersion);
    HEPHAESTUS_VK_DISPATCHER_LOAD_GLOBAL_FUNCTION(vkEnumerateInstanceExtensionProperties);
    HEPHAESTUS_VK_DISPATCHER_LOAD_GLOBAL_FUNCTION(vkEnumerateInstanceLayerProperties);
}

void 
VulkanDispatcher::LoadInstanceFunctions(const vk::Instance& instance)
{
    HEPHAESTUS_LOG_ASSERT(s_dispatcherInstance.vkGetInstanceProcAddr, "Dispatcher has not been initialized");

    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkEnumeratePhysicalDevices, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceProperties, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceFeatures, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateDevice, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetDeviceProcAddr, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyInstance, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceMemoryProperties, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkEnumerateDeviceExtensionProperties, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroySurfaceKHR, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateDebugUtilsMessengerEXT, instance);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyDebugUtilsMessengerEXT, instance);

#ifdef HEPHAESTUS_PLATFORM_WIN32
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateWin32SurfaceKHR, instance); // win32
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateXlibSurfaceKHR, instance);
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateAndroidSurfaceKHR, instance);
#else
    static_assert(false, "VulkanFunctionDispatcher: Unknown platform configuration");
#endif
}

void 
VulkanDispatcher::LoadDeviceFunctions(const vk::Device& device)
{
    HEPHAESTUS_LOG_ASSERT(s_dispatcherInstance.vkGetInstanceProcAddr, "Dispatcher has not been initialized");

    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetDeviceQueue,device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDeviceWaitIdle, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyDevice, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateSemaphore, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateCommandPool, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkAllocateCommandBuffers, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkBeginCommandBuffer, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdPipelineBarrier, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdClearColorImage, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkEndCommandBuffer, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkQueueSubmit, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkFreeCommandBuffers, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyCommandPool, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroySemaphore, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateSwapchainKHR, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetSwapchainImagesKHR, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkAcquireNextImageKHR, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkQueuePresentKHR, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroySwapchainKHR, device);

    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateImage, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateImageView, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateRenderPass, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateFramebuffer, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateShaderModule, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreatePipelineLayout, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateGraphicsPipelines, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdBeginRenderPass, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdBindPipeline, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdDraw, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdEndRenderPass, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyShaderModule, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyPipelineLayout, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyPipeline, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyRenderPass, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyFramebuffer, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyImageView, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyImage, device);

    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateFence, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateBuffer, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetBufferMemoryRequirements, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkAllocateMemory, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkBindBufferMemory, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkMapMemory, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkFlushMappedMemoryRanges, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkUnmapMemory, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdSetViewport, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdSetScissor, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdBindVertexBuffers, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkWaitForFences, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkResetFences, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkFreeMemory, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyBuffer, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyFence, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdCopyBuffer, device);

    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetImageMemoryRequirements, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkBindImageMemory, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateSampler, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdCopyBufferToImage, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateDescriptorSetLayout, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCreateDescriptorPool, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkAllocateDescriptorSets, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkUpdateDescriptorSets, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdBindDescriptorSets, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyDescriptorPool, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroyDescriptorSetLayout, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkDestroySampler, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkFreeDescriptorSets, device);

    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdBindIndexBuffer, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdDrawIndexed, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdPushConstants, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkResetCommandPool, device);

    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdSetLineWidth, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkCmdCopyImage, device);
    HEPHAESTUS_VK_DISPATCHER_LOAD_OBJECT_FUNCTION(vkGetImageSubresourceLayout, device);
}

}