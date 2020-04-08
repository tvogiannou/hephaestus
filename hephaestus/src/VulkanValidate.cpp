#include <hephaestus/VulkanValidate.h>

#include <hephaestus/Log.h>
#include <hephaestus/Compiler.h>
#include <hephaestus/VulkanDispatcher.h>
#include <hephaestus/VulkanConfig.h>
#include <hephaestus/VulkanUtils.h>

#include <vector>
#include <cstring>


namespace hephaestus
{

// static
bool 
VulkanValidate::CheckInstanceValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    std::vector<vk::LayerProperties> availableLayers;
    HEPHAESTUS_CHECK_RESULT_RAW(availableLayers, vk::enumerateInstanceLayerProperties());

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
VulkanValidate::CheckInstanceRequiredExtensions(const std::vector<const char*>& instanceExtensions)
{
    std::vector<vk::ExtensionProperties> instanceExtensionProperties;
    HEPHAESTUS_CHECK_RESULT_RAW(instanceExtensionProperties, vk::enumerateInstanceExtensionProperties(nullptr));

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
    const vk::PhysicalDevice& physicalDevice)
{
    HEPHAESTUS_LOG_ASSERT(physicalDevice, "No Vulkan physical device available");
    
    std::vector<vk::ExtensionProperties> instanceExtensionProperties;
    HEPHAESTUS_CHECK_RESULT_RAW(instanceExtensionProperties, 
        physicalDevice.enumerateDeviceExtensionProperties(nullptr));

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
    const vk::PhysicalDevice& physicalDevice)
{
    HEPHAESTUS_LOG_ASSERT(physicalDevice, "No Vulkan physical device available");

    const vk::PhysicalDeviceProperties& properties = physicalDevice.getProperties();
    //const vk::PhysicalDeviceFeatures& features = physicalDevice.getFeatures();

    const uint32_t major_version = VK_VERSION_MAJOR( properties.apiVersion );
    if( (major_version < 1) || (properties.limits.maxImageDimension2D < 4096) )
    {
        HEPHAESTUS_LOG_ERROR("No support for required Vulkan features");
        return false;
    }

    return true;
}


}