#include <hephaestus/VulkanDeviceManager.h>

#include <hephaestus/Log.h>
#include <hephaestus/VulkanDispatcher.h>
#include <hephaestus/VulkanValidate.h>


namespace hephaestus
{

static 
VKAPI_ATTR VkBool32 VKAPI_CALL s_DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/) {

    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            HEPHAESTUS_LOG_ERROR("vk::SystemError: %s", pCallbackData->pMessage);
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            HEPHAESTUS_LOG_WARNING("vk::SystemWarning: %s", pCallbackData->pMessage);
// INFO_BIT is generating a bit too much output noise
// 		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
// 			HEPHAESTUS_LOG_INFO("vk::SystemError: %s", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

std::vector<char const*>
VulkanDeviceManager::GetDeviceRequiredExtensions()
{
    std::vector<char const*> extensions;

    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return extensions;
}

std::vector<char const*>
VulkanDeviceManager::GetInstanceRequiredExtensions(bool enableValidationLayers)
{
    std::vector<char const*> extensions;

    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef HEPHAESTUS_PLATFORM_WIN32
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME); // win32
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}


bool
VulkanDeviceManager::CreateInstance(bool enableValidationLayers)
{
    const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
    
    const std::vector<const char*>& instanceExtensions =
        GetInstanceRequiredExtensions(enableValidationLayers);
    HEPHAESTUS_LOG_ASSERT(VulkanValidate::CheckInstanceRequiredExtensions(instanceExtensions), 
        "No support for required Vulkan extensions");

    // TODO: expose names as argument
    vk::ApplicationInfo appInfo("test", 1, "hephaestus engine", 1, VK_API_VERSION_1_1);
    vk::InstanceCreateInfo creationInfo(vk::InstanceCreateFlags(), &appInfo);
    creationInfo.setEnabledExtensionCount((uint32_t)instanceExtensions.size());
    creationInfo.setPpEnabledExtensionNames(instanceExtensions.data());
    if (enableValidationLayers)
    {
        HEPHAESTUS_LOG_ASSERT(VulkanValidate::CheckInstanceValidationLayerSupport(validationLayers),
            "No support for required Vulkan layer extensions");
        creationInfo.setEnabledLayerCount((uint32_t)validationLayers.size());
        creationInfo.setPpEnabledLayerNames(validationLayers.data());
    }
    m_instance = vk::createInstanceUnique(creationInfo, nullptr);

    VulkanDispatcher::GetInstance().LoadInstanceFunctions(m_instance.get());


    if (enableValidationLayers)
    {
        vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
        messengerCreateInfo.messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        messengerCreateInfo.messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        messengerCreateInfo.pfnUserCallback = s_DebugCallback;
        messengerCreateInfo.pUserData = nullptr;

        m_debugMessenger = m_instance->createDebugUtilsMessengerEXTUnique(messengerCreateInfo, nullptr);
    }

    return true;
}

bool 
VulkanDeviceManager::CreateDevice(bool createPresentQueue /*= true*/)
{
    HEPHAESTUS_LOG_ASSERT(m_instance, "Vulkan instance has not been initialized");

    const std::vector<vk::PhysicalDevice>& physicalDevices = m_instance->enumeratePhysicalDevices();
    if (physicalDevices.empty())
        return false;

    // TODO: pick the physical context.device with the desired extensions
    m_physicalDevice = physicalDevices[0];

    HEPHAESTUS_LOG_ASSERT(VulkanValidate::CheckPhysicalDevicePropertiesAndFeatures(m_physicalDevice),
        "No support for Vulkan physical device required properties and features");
    const std::vector<const char*>& deviceExtensions = GetDeviceRequiredExtensions();
    HEPHAESTUS_LOG_ASSERT(VulkanValidate::CheckPhysicalDeviceRequiredExtensions(deviceExtensions, m_physicalDevice),
        "No support for Vulkan physical device required extensions");

    if (!SetupQueueFamilies(createPresentQueue))
        return false;

    float queuePriority = 0.0f;
    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(), m_graphicsQueueInfo.familyIndex, 1, &queuePriority));
    if (createPresentQueue && m_presentQueueInfo.familyIndex != m_graphicsQueueInfo.familyIndex)
    {
        deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags(), m_presentQueueInfo.familyIndex, 1, &queuePriority));
    }

    vk::DeviceCreateInfo deviceCreateInfo(
        vk::DeviceCreateFlags(), 
        (uint32_t)deviceQueueCreateInfos.size(), 
        deviceQueueCreateInfos.data());
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    m_device = m_physicalDevice.createDeviceUnique(deviceCreateInfo, nullptr);

    VulkanDispatcher::GetInstance().LoadDeviceFunctions(m_device.get());

    return true;
}

bool 
VulkanDeviceManager::CreateQueues(bool createPresentQueue /*= true*/)
{
    HEPHAESTUS_LOG_ASSERT(m_device, "Vulkan device has not been initialized");

    m_graphicsQueueInfo.queue = m_device->getQueue(m_graphicsQueueInfo.familyIndex, 0);
    if (createPresentQueue)
        m_presentQueueInfo.queue = m_device->getQueue(m_presentQueueInfo.familyIndex, 0);

    return true;
}

bool 
VulkanDeviceManager::SetupQueueFamilies(bool findPresentQueue /*= true*/)
{
    // get the QueueFamilyProperties of the first PhysicalDevice
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    // try to find a Queue family which supports both graphics & present
    m_graphicsQueueInfo.familyIndex = m_presentQueueInfo.familyIndex = VulkanUtils::InvalidQueueIndex;
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size(); ++queueFamilyIndex)
    {
        const vk::QueueFamilyProperties& qfp = queueFamilyProperties[queueFamilyIndex];
        if (qfp.queueFlags & vk::QueueFlagBits::eGraphics)
            m_graphicsQueueInfo.familyIndex = /*m_presentQueueInfo.familyIndex =*/ queueFamilyIndex;

        if (findPresentQueue)
        {
            if (m_physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, m_presentSurface.get()))
                m_presentQueueInfo.familyIndex = queueFamilyIndex;

            if (m_graphicsQueueInfo.familyIndex < queueFamilyProperties.size() &&
                m_presentQueueInfo.familyIndex < queueFamilyProperties.size())
            {
                // since we always set the latest supported queue, if a queue supports both it will have set
                // both of the indices to the same queue family index so we can stop looking
                if (m_graphicsQueueInfo.familyIndex == m_presentQueueInfo.familyIndex)
                    break;
            }
        }
        else if (m_graphicsQueueInfo.familyIndex < queueFamilyProperties.size())
            break;
    }

    bool res = findPresentQueue ?
        m_graphicsQueueInfo.familyIndex < queueFamilyProperties.size() &&
        m_presentQueueInfo.familyIndex < queueFamilyProperties.size() :
        m_graphicsQueueInfo.familyIndex < queueFamilyProperties.size();

    return res;
}

void 
VulkanDeviceManager::WaitDevice() const
{
    if (m_device)
        m_device->waitIdle();
}

void 
VulkanDeviceManager::Clear()
{
    m_presentSurface.reset(nullptr);
    m_device.reset(nullptr);
    m_instance.reset(nullptr);
}

bool 
VulkanDeviceManager::Init(const PlatformWindowInfo& windowInfo, bool enableValidationLayers /*= true*/)
{
    const bool createPresentQueue =
#ifdef HEPHAESTUS_PLATFORM_WIN32
        windowInfo.instance != NULL;
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
        windowInfo.display != nullptr;
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
        windowInfo.platformWindow != nullptr;
#endif 

    if (!CreateInstance(enableValidationLayers))
        return false;

    if (createPresentQueue)
    {
#ifdef HEPHAESTUS_PLATFORM_WIN32
        if (!CreatePresentSurface(windowInfo.instance, windowInfo.handle))
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
        if (!CreatePresentSurface(windowInfo.display, windowInfo.handle))
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
        if (!CreatePresentSurface(windowInfo.platformWindow))
#endif
            return false;
    }

    if (!CreateDevice(createPresentQueue))
        return false;

    if (!CreateQueues(createPresentQueue))
        return false;

    return true;
}

#ifdef HEPHAESTUS_PLATFORM_WIN32
bool
VulkanDeviceManager::CreatePresentSurface(HINSTANCE instance, HWND handle)
{
    HEPHAESTUS_LOG_ASSERT(m_instance, "Vulkan instance has not been initialized");
    HEPHAESTUS_LOG_ASSERT(instance != nullptr, "Win32 instance has not been initialized");
    HEPHAESTUS_LOG_ASSERT(handle != nullptr, "Win32 handle has not been initialized");

    vk::Win32SurfaceCreateInfoKHR surfaceInfo(
        vk::Win32SurfaceCreateFlagsKHR(), instance, handle);
    m_presentSurface = m_instance->createWin32SurfaceKHRUnique(surfaceInfo, nullptr);

    return true;
}
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
bool
VulkanDeviceManager::CreatePresentSurface(Display* display, Window handle)
{
    HEPHAESTUS_LOG_ASSERT(m_instance, "Vulkan instance has not been initialized");
    HEPHAESTUS_LOG_ASSERT(display != nullptr, "Xlib display has not been initialized");
    //HEPHAESTUS_LOG_ASSERT(handle != nullptr, "Xlib handle has not been initialized");

    vk::XlibSurfaceCreateInfoKHR surfaceInfo(
        vk::XlibSurfaceCreateFlagsKHR(), display, handle);
    m_presentSurface = m_instance->createXlibSurfaceKHRUnique(surfaceInfo, nullptr);

    return true;
}
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
bool 
VulkanDeviceManager::CreatePresentSurface(ANativeWindow* platformWindow)
{
    HEPHAESTUS_LOG_ASSERT(m_instance, "Vulkan instance has not been initialized");
    HEPHAESTUS_LOG_ASSERT(platformWindow != nullptr, "Android native window has not been initialized");

    vk::AndroidSurfaceCreateInfoKHR surfaceInfo(
        vk::AndroidSurfaceCreateFlagsKHR(),
        platformWindow);
    m_presentSurface = m_instance->createAndroidSurfaceKHRUnique(surfaceInfo, nullptr);

    return true;
}
#endif

} // namespace hephaestus