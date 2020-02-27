#pragma once

#include <vector>


namespace vk
{
class PhysicalDevice;
}

namespace hephaestus
{
// Utility methods for checking required Vulkan extensions and features
struct VulkanValidate
{
    static bool CheckInstanceValidationLayerSupport(const std::vector<const char*>& validationLayers);
    static bool CheckInstanceRequiredExtensions(const std::vector<const char*>& instanceExtensions);
    static bool CheckPhysicalDevicePropertiesAndFeatures(const vk::PhysicalDevice& physicalDevice);
    static bool CheckPhysicalDeviceRequiredExtensions(
        const std::vector<const char*>& deviceExtensions, 
        const vk::PhysicalDevice& physicalDevice);
};

} // hephaestus