#pragma once

#include <hephaestus/Compiler.h>
#include <hephaestus/VulkanConfig.h>
#include <hephaestus/VulkanUtils.h>

#include <vector>


namespace hephaestus
{
// Class for dealing with the core management of the Vulkan device
// - container for Vulkan device, instance & queues
// - optionally setups the present surface (extension)
class VulkanDeviceManager
{
public:
    static std::vector<char const*> GetDeviceRequiredExtensions();
    static std::vector<char const*> GetInstanceRequiredExtensions(bool enableValidationLayers);

public:
    VulkanDeviceManager() = default;
    ~VulkanDeviceManager() { WaitDevice(); }

    struct PlatformWindowInfo
    {
#ifdef HEPHAESTUS_PLATFORM_WIN32
        HINSTANCE instance = NULL;
        HWND handle;
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
        Display* display = nullptr; 
        Window handle;
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
        ANativeWindow* platformWindow = nullptr;
#endif
    };
    bool Init(const PlatformWindowInfo& windowInfo, bool enableValidationLayers = true);

    void WaitDevice() const;
    void Clear();

    const vk::Instance& GetInstance() const { return m_instance.get(); }
    const vk::Device& GetDevice() const { return m_device.get(); }
    const vk::PhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; }
    const vk::SurfaceKHR& GetPresentSurface() const { return m_presentSurface.get(); }
    const VulkanUtils::QueueInfo& GetGraphicsQueueInfo() const { return m_graphicsQueueInfo; }
    const VulkanUtils::QueueInfo& GetPresentQueueInfo() const { return m_presentQueueInfo; }

    vk::Instance GetInstance() { return m_instance.get(); }
    vk::Device GetDevice() { return m_device.get(); }
    vk::PhysicalDevice GetPhysicalDevice() { return m_physicalDevice; }
    VulkanUtils::QueueInfo GetGraphicsQueueInfo() { return m_graphicsQueueInfo; }
    VulkanUtils::QueueInfo GetPresentQueueInfo() { return m_presentQueueInfo; }

private:
    // internal helpers
    bool CreateInstance(bool enableValidationLayers);
    bool CreateDevice(bool createPresentQueue = true);
    bool CreateQueues(bool createPresentQueue = true);
    bool SetupQueueFamilies(bool findPresentQueue = true);
#ifdef HEPHAESTUS_PLATFORM_WIN32
    bool CreatePresentSurface(HINSTANCE instance, HWND handle);
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    bool CreatePresentSurface(Display* display, Window handle);
#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    bool CreatePresentSurface(ANativeWindow* platformWindow);
#endif

private:
    vk::UniqueHandle<vk::Instance, VulkanDispatcher>    m_instance;
    vk::UniqueHandle<vk::Device, VulkanDispatcher>      m_device;
    vk::PhysicalDevice                                  m_physicalDevice;
    vk::UniqueHandle<vk::SurfaceKHR, VulkanDispatcher>  m_presentSurface;
    VulkanUtils::QueueInfo                              m_graphicsQueueInfo;
    VulkanUtils::QueueInfo                              m_presentQueueInfo;

    // debugging
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, VulkanDispatcher> m_debugMessenger;
};

} // hephaestus