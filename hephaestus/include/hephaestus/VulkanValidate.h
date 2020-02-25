#pragma once

#include <vector>


namespace vk
{
class PhysicalDevice;
}

namespace hephaestus
{

// Utils class with methods to check for required Vulkan extensions and features
class VulkanValidate
{
public:

    static bool CheckInstanceValidationLayerSupport(const std::vector<const char*>& validationLayers);
    static bool CheckInstanceRequiredExtensions(const std::vector<const char*>& instanceExtensions);
    static bool CheckPhysicalDevicePropertiesAndFeatures(const vk::PhysicalDevice& physicalDevice);
    static bool CheckPhysicalDeviceRequiredExtensions(
        const std::vector<const char*>& deviceExtensions, 
        const vk::PhysicalDevice& physicalDevice);
};

}