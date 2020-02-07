#pragma once

#include <vector>


namespace vk
{
class PhysicalDevice;
}

namespace hephaestus
{
class VulkanDispatcher;

// Utils class with methods to check for required Vulkan extensions and features
class VulkanValidate
{
public:

	static bool checkInstanceValidationLayerSupport(
		const std::vector<const char*>& validationLayers, 
		const hephaestus::VulkanDispatcher& dispatcher);
	static bool checkInstanceRequiredExtensions(
		const std::vector<const char*>& instanceExtensions, 
		const hephaestus::VulkanDispatcher& dispatcher);
	static bool checkPhysicalDeviceRequiredExtensions(
		const std::vector<const char*>& deviceExtensions, 
		const vk::PhysicalDevice& physicalDevice,
		const hephaestus::VulkanDispatcher& dispatcher);
	static bool checkPhysicalDevicePropertiesAndFeatures(
		const vk::PhysicalDevice& physicalDevice,
		const hephaestus::VulkanDispatcher& dispatcher);
};

}