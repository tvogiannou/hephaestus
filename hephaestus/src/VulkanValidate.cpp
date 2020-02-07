#include <hephaestus/VulkanValidate.h>

#include <hephaestus/Log.h>
#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDispatcher.h>
#include <hephaestus/VulkanPlatformConfig.h>

#include <vector>
#include <cstring>


namespace hephaestus
{

// static
bool 
VulkanValidate::CheckInstanceValidationLayerSupport(
    const std::vector<const char*>& validationLayers, 
    const VulkanDispatcher& dispatcher)
{
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties(dispatcher);

    for (const char* layerName : validationLayers)
    {
        auto it = std::find_if(availableLayers.begin(), availableLayers.end(),
            [layerName](const vk::LayerProperties availableLayer)
            {
                return std::strcmp(layerName, availableLayer.layerName) == 0;
            }
        );
        if (it == availableLayers.end())
        {
            HEPHAESTUS_LOG_ERROR("No support for required Vulkan layer %s", layerName);
            return false;
        }
    }

    return true;
}

//static 
bool 
VulkanValidate::CheckInstanceRequiredExtensions(
    const std::vector<const char*>& instanceExtensions, 
    const VulkanDispatcher& dispatcher)
{
    std::vector<vk::ExtensionProperties> instanceExtensionProperties = 
                        vk::enumerateInstanceExtensionProperties(nullptr, dispatcher);

    for (const char* extensionName : instanceExtensions)
    {
        bool extensionSupported = false;
        for (const vk::ExtensionProperties& extensionProperty : instanceExtensionProperties)
        {
            if (std::strcmp(extensionName, extensionProperty.extensionName) == 0)
            {
                extensionSupported = true;
                break;
            }
        }

        if (!extensionSupported)
        {
            HEPHAESTUS_LOG_ERROR("No support for required Vulkan instance extension %s", extensionName);
            return false;
        }
    }

    return true;
}

//static 
bool 
VulkanValidate::CheckPhysicalDeviceRequiredExtensions(
    const std::vector<const char*>& deviceExtensions,
    const vk::PhysicalDevice& physicalDevice,
    const VulkanDispatcher& dispatcher)
{
    HEPHAESTUS_LOG_ASSERT(physicalDevice, "No Vulkan physical device available");
    
    std::vector<vk::ExtensionProperties> instanceExtensionProperties =
        physicalDevice.enumerateDeviceExtensionProperties(nullptr, dispatcher);

    for (const char* extensionName : deviceExtensions)
    {
        bool extensionSupported = false;
        for (const vk::ExtensionProperties& extensionProperty : instanceExtensionProperties)
        {
            if (std::strcmp(extensionName, extensionProperty.extensionName) == 0)
            {
                extensionSupported = true;
                break;
            }
        }

        if (!extensionSupported)
        {
            HEPHAESTUS_LOG_ERROR("No support for required Vulkan device extension %s", extensionName);
            return false;
        }
    }

    return true;
}

//static 
bool 
VulkanValidate::CheckPhysicalDevicePropertiesAndFeatures(
    const vk::PhysicalDevice& physicalDevice, 
    const VulkanDispatcher& dispatcher)
{
    HEPHAESTUS_LOG_ASSERT(physicalDevice, "No Vulkan physical device available");

    const vk::PhysicalDeviceProperties& properties = physicalDevice.getProperties(dispatcher);
    //const vk::PhysicalDeviceFeatures& features = physicalDevice.getFeatures(dispatcher);

    const uint32_t major_version = VK_VERSION_MAJOR( properties.apiVersion );
    if( (major_version < 1) || (properties.limits.maxImageDimension2D < 4096) )
    {
        HEPHAESTUS_LOG_ERROR("No support for required Vulkan features");
        return false;
    }

    return true;
}


}