#pragma once

#include <vector>


namespace vk
{
class PhysicalDevice;
}

namespace hephaestus
{
struct VulkanDispatcher;

// Utils class with methods to check for required Vulkan extensions and features
class VulkanValidate
{
public:

    static bool CheckInstanceValidationLayerSupport(
        const std::vector<const char*>& validationLayers, 
        const hephaestus::VulkanDispatcher& dispatcher);
    static bool CheckInstanceRequiredExtensions(
        const std::vector<const char*>& instanceExtensions, 
        const hephaestus::VulkanDispatcher& dispatcher);
    static bool CheckPhysicalDeviceRequiredExtensions(
        const std::vector<const char*>& deviceExtensions, 
        const vk::PhysicalDevice& physicalDevice,
        const hephaestus::VulkanDispatcher& dispatcher);
    static bool CheckPhysicalDevicePropertiesAndFeatures(
        const vk::PhysicalDevice& physicalDevice,
        const hephaestus::VulkanDispatcher& dispatcher);
};

}