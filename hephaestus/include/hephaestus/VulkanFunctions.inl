
#ifndef VULKAN_EXPORTEDFUNCTION_DECLARATION
#define VULKAN_EXPORTEDFUNCTION_DECLARATION(fun) extern PFN_##fun fun
#endif

// global functions 
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateInstance);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkEnumerateInstanceVersion);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkEnumerateInstanceExtensionProperties);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkEnumerateInstanceLayerProperties);

// instance functions
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkEnumeratePhysicalDevices);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceProperties);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceFeatures);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceQueueFamilyProperties);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateDevice);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetDeviceProcAddr);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyInstance);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceMemoryProperties);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkEnumerateDeviceExtensionProperties);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceSurfaceSupportKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceSurfaceFormatsKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetPhysicalDeviceSurfacePresentModesKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroySurfaceKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateDebugUtilsMessengerEXT);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyDebugUtilsMessengerEXT);
#ifdef HEPHAESTUS_PLATFORM_WIN32
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateWin32SurfaceKHR); // win32
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateXlibSurfaceKHR); // xlib
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateAndroidSurfaceKHR); // android
#endif

// device functions
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetDeviceQueue);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDeviceWaitIdle);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyDevice);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateSemaphore);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateCommandPool);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkAllocateCommandBuffers);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkBeginCommandBuffer);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdPipelineBarrier);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdClearColorImage);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkEndCommandBuffer);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkQueueSubmit);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkFreeCommandBuffers);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyCommandPool);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroySemaphore);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateSwapchainKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetSwapchainImagesKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkAcquireNextImageKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkQueuePresentKHR);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroySwapchainKHR);

VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateImage);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateImageView);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateRenderPass);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateFramebuffer);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateShaderModule);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreatePipelineLayout);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateGraphicsPipelines);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdBeginRenderPass);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdBindPipeline);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdDraw);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdEndRenderPass);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyShaderModule);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyPipelineLayout);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyPipeline);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyRenderPass);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyFramebuffer);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyImageView);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyImage);

VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateFence);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateBuffer);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetBufferMemoryRequirements);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkAllocateMemory);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkBindBufferMemory);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkMapMemory);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkFlushMappedMemoryRanges);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkUnmapMemory);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdSetViewport);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdSetScissor);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdBindVertexBuffers);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkWaitForFences);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkResetFences);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkFreeMemory);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyBuffer);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyFence);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdCopyBuffer);

VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetImageMemoryRequirements);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkBindImageMemory);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateSampler);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdCopyBufferToImage);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateDescriptorSetLayout);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCreateDescriptorPool);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkAllocateDescriptorSets);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkUpdateDescriptorSets);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdBindDescriptorSets);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyDescriptorPool);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroyDescriptorSetLayout);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkDestroySampler);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkFreeDescriptorSets);

VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdBindIndexBuffer);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdDrawIndexed);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdPushConstants);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkResetCommandPool);

VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdSetLineWidth);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkCmdCopyImage);
VULKAN_EXPORTEDFUNCTION_DECLARATION(vkGetImageSubresourceLayout);