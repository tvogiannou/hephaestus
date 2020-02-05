#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanFunctionDispatcher.h>
#include <hephaestus/VulkanPlatformConfig.h>

#include <vector>


namespace hephaestus
{

class VulkanDeviceManager;

// Utilities for handling Vulkan objects
struct VulkanUtils
{
public:
    // Util types

    // Handler types for Vulkan objects
    using ShaderModuleHandle = vk::UniqueHandle<vk::ShaderModule, hephaestus::VulkanFunctionDispatcher>;
    using DescriptorPoolHandle = vk::UniqueHandle<vk::DescriptorPool, hephaestus::VulkanFunctionDispatcher>;
    using RenderPassHandle = vk::UniqueHandle<vk::RenderPass, hephaestus::VulkanFunctionDispatcher>;
    using CommandBufferHandle = vk::UniqueHandle<vk::CommandBuffer, hephaestus::VulkanFunctionDispatcher>;
    using SwapChainHandle = vk::UniqueHandle<vk::SwapchainKHR, hephaestus::VulkanFunctionDispatcher>;
    using ImageViewHandle = vk::UniqueHandle<vk::ImageView, hephaestus::VulkanFunctionDispatcher>;
    using FramebufferHandle = vk::UniqueHandle<vk::Framebuffer, hephaestus::VulkanFunctionDispatcher>;
    using FenceHandle = vk::UniqueHandle<vk::Fence, hephaestus::VulkanFunctionDispatcher>;
    using SemaphoreHandle = vk::UniqueHandle<vk::Semaphore, hephaestus::VulkanFunctionDispatcher>;
    using DescriptorSetHandle = vk::UniqueHandle<vk::DescriptorSet, hephaestus::VulkanFunctionDispatcher>;
    using BufferHandle = vk::UniqueHandle<vk::Buffer, hephaestus::VulkanFunctionDispatcher>;
    using DeviceMemoryHandle = vk::UniqueHandle<vk::DeviceMemory, hephaestus::VulkanFunctionDispatcher>;
    using ImageHandle = vk::UniqueHandle<vk::Image, hephaestus::VulkanFunctionDispatcher>;
    using SamplerHandle = vk::UniqueHandle<vk::Sampler, hephaestus::VulkanFunctionDispatcher>;
    using CommandPoolHandle = vk::UniqueHandle<vk::CommandPool, hephaestus::VulkanFunctionDispatcher>;
    using PipelineHandle = vk::UniqueHandle<vk::Pipeline, hephaestus::VulkanFunctionDispatcher>;
    using PipelineLayoutHandle = vk::UniqueHandle<vk::PipelineLayout, hephaestus::VulkanFunctionDispatcher>;
    using DescriptorSetLayoutHandle = vk::UniqueHandle<vk::DescriptorSetLayout, hephaestus::VulkanFunctionDispatcher>;

    // Container with info for recording draw commands during a frame update
    struct FrameUpdateInfo
    {
        vk::CommandBuffer   drawCmdBuffer;
        vk::Framebuffer     framebuffer;
        vk::Image           image;
        vk::ImageView       view;
        vk::Extent2D        extent;
        vk::RenderPass      renderPass;
    };

    // Container for keeping track of the Swap Chain
    struct SwapChainInfo
    {
        // utility container for referencing images owned by the swap chain
        struct ImageRef
        {
            ImageRef(vk::Image _imageHandle) : image(_imageHandle) {}

            vk::Image           image; // this imaged is owned by the swap chain
            ImageViewHandle     view;
        };

        SwapChainHandle             swapChainHandle;
        vk::Format                  format = vk::Format::eUndefined;
        vk::Extent2D                extent;
        std::vector<ImageRef>	    imageRefs;
    };

    // Container for the resources used per "virtual frame", i.e. the operations needed to render a single frame
    struct VirtualFrameResources
    {
        FramebufferHandle     framebuffer;	// framebuffer to store target image during command buffer processing
        CommandBufferHandle   commandBuffer;
        SemaphoreHandle       imageAvailableSemaphore;
        SemaphoreHandle       finishedRenderingSemaphore;
        FenceHandle           fence;

        void Clear()
        {
            fence.reset(nullptr);
            finishedRenderingSemaphore.reset(nullptr);
            imageAvailableSemaphore.reset(nullptr);
            commandBuffer.reset(nullptr);
            framebuffer.reset(nullptr);
        }
    };

    // Container for handling Descriptor Sets
    struct DescriptorSetInfo
    {
        DescriptorSetHandle        handle;

        void Clear()
        {
            handle.reset(nullptr);
        }
    };

    // Container for handling Buffers
    struct BufferInfo
    {
        BufferHandle        bufferHandle;
        DeviceMemoryHandle  deviceMemory;
        uint32_t            size = 0;

        bool IsValid() const
        {
            return bufferHandle && deviceMemory && size > 0;
        }

        void Clear()
        {
            deviceMemory.reset(nullptr);
            bufferHandle.reset(nullptr);
        }
    };

    // Container for images owned by the app
    struct ImageInfo
    {
        ImageHandle         imageHandle;
        ImageViewHandle     view;
        SamplerHandle       sampler;
        DeviceMemoryHandle  deviceMemory;

        void Clear()
        {
            view.reset(nullptr);
            sampler.reset(nullptr);
            deviceMemory.reset(nullptr);
            imageHandle.reset(nullptr);
        }
    };

    struct BufferUpdateInfo
    {
        vk::CommandBuffer   copyCmdBuffer;
        const char*         data;
        uint32_t            dataSize;
    };

    struct TextureUpdateInfo : public BufferUpdateInfo
    {
        uint32_t width;
        uint32_t height;
    };

    // Util type to help with selecting shaders for pipelines
    struct ShaderDB
    {
        using ShaderDBIndex = size_t;

        static const uint32_t MAX_SHADERS = 32u;

        vk::ShaderModule GetModule(ShaderDBIndex index) const { return loadedShaders.at(index).get(); }

        std::array<VulkanUtils::ShaderModuleHandle, MAX_SHADERS> loadedShaders;
    };

public:
    // Utils methods

    static ShaderModuleHandle CreateShaderModule(const VulkanDeviceManager& deviceManager, 
        const char* filename);

    static bool CreateDepthImage(const VulkanDeviceManager& deviceManager, 
        uint32_t width, uint32_t height, ImageInfo& depthImageInfo);

    static bool CreateBuffer(const VulkanDeviceManager& deviceManager, uint32_t size, 
        vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperty, BufferInfo& bufferInfo);

    static bool AllocateBufferMemory(const VulkanDeviceManager& deviceManager, 
        vk::MemoryPropertyFlags requiredMemoryProperty, BufferInfo& bufferInfo);

    static bool AllocateImageMemory(const VulkanDeviceManager& deviceManager, 
        vk::MemoryPropertyFlags requiredMemoryProperty, ImageInfo& imageInfo);

    static bool CreateImageTextureInfo(const VulkanDeviceManager& deviceManager, uint32_t width, uint32_t height, 
        ImageInfo& textureInfo);

    static bool CopyImageDataStage(const VulkanDeviceManager& deviceManager, const BufferInfo &stageBufferInfo, 
        const ImageInfo& imageInfo, const TextureUpdateInfo& textureUpdateInfo);

    // Copy data to (device local) buffer using the staging buffer
    static bool CopyBufferDataStage(
        const VulkanDeviceManager& deviceManager, const BufferInfo &stageBufferInfo,
        const BufferUpdateInfo &updateInfo, const BufferInfo& dstBufferInfo,
        vk::AccessFlagBits dstFinalAccessMask, vk::PipelineStageFlagBits dstStageMask, VkDeviceSize dstBufferOffset = 0u);

    // Copy data to (host local) buffer
    static bool CopyBufferDataHost(const VulkanDeviceManager& deviceManager, 
        const BufferUpdateInfo &updateInfo, const BufferInfo& dstBufferInfo, VkDeviceSize dstBufferOffset = 0u);

    static bool CreateDescriptorPool(const VulkanDeviceManager& deviceManager,
        uint32_t uniformSize, uint32_t combinedImgSamplerSize, DescriptorPoolHandle& descriptorPool);

    static bool CreateRenderPass(const VulkanDeviceManager& deviceManager, 
        vk::Format format, vk::ImageLayout finalLayout, RenderPassHandle& renderPass);

    static uint32_t FixupFlushRange(const VulkanDeviceManager& deviceManager, uint32_t size);
};

}