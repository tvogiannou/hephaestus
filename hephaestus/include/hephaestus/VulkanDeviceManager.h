#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanPlatformConfig.h>
#include <hephaestus/VulkanDispatcher.h>

#include <vector>


namespace hephaestus
{

class VulkanDeviceManager
{
public:
    static const uint32_t InvalidQueueIndex = UINT32_MAX;

    struct QueueInfo
    {
        vk::Queue	queue = nullptr;
        uint32_t	familyIndex = VulkanDeviceManager::InvalidQueueIndex;
    };

    static std::vector<char const*> GetDeviceRequiredExtensions();
    static std::vector<char const*> GetInstanceRequiredExtensions(bool enableValidationLayers);

public:
    explicit VulkanDeviceManager(VulkanDispatcher& _dispatcher) :
        m_dispatcher(_dispatcher)
    {}

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

    const VulkanDispatcher& GetDispatcher() const { return m_dispatcher; }

    const vk::Instance& GetInstance() const { return m_instance.get(); }
    const vk::Device& GetDevice() const { return m_device.get(); }
    const vk::PhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; }
    const vk::SurfaceKHR& GetPresentSurface() const { return m_presentSurface.get(); }
    const QueueInfo& GetGraphicsQueueInfo() const { return m_graphicsQueueInfo; }
    const QueueInfo& GetPresentQueueInfo() const { return m_presentQueueInfo; }

    vk::Instance GetInstance() { return m_instance.get(); }
    vk::Device GetDevice() { return m_device.get(); }
    vk::PhysicalDevice GetPhysicalDevice() { return m_physicalDevice; }
    QueueInfo GetGraphicsQueueInfo() { return m_graphicsQueueInfo; }
    QueueInfo GetPresentQueueInfo() { return m_presentQueueInfo; }

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
    VulkanDispatcher& m_dispatcher;

    vk::UniqueHandle<vk::Instance, VulkanDispatcher> m_instance;
    vk::UniqueHandle<vk::Device, VulkanDispatcher> m_device;
    vk::PhysicalDevice m_physicalDevice;
    vk::UniqueHandle<vk::SurfaceKHR, VulkanDispatcher> m_presentSurface;
    QueueInfo m_graphicsQueueInfo;
    QueueInfo m_presentQueueInfo;

    // debugging
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, VulkanDispatcher> m_debugMessenger;
};

}