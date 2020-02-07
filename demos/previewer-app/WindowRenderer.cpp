#include "WindowRenderer.h"

#include "imgui_impl_vulkan.h"
#include <hephaestus/Log.h>
#include <common/Camera.h>
#include <common/CommonUtils.h>
#include <common/Matrix4.h>
#include <hephaestus/PipelineBase.h>
#include <hephaestus/VulkanPlatformConfig.h>


static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;

    HEPHAESTUS_LOG_ERROR("vk::SystemError: %u", err);

    if (err < 0)
        std::exit(EXIT_FAILURE);
}

namespace hephaestus
{

bool
VulkanWindowRenderer::ImGuiPipeline::SetupPipeline(
    vk::RenderPass renderPass, vk::CommandPool commandPool, vk::CommandBuffer cmdBuffer)
{
    constexpr uint32_t uniformSize = 4u;
    constexpr uint32_t combinedImgSamplerSize = 4u;
    if (!VulkanUtils::CreateDescriptorPool(
        m_deviceManager, uniformSize, combinedImgSamplerSize, m_descriptorPool))
        return false;

    // Setup Platform/Renderer bindings
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VkInstance(m_deviceManager.GetInstance());
    init_info.PhysicalDevice = VkPhysicalDevice(m_deviceManager.GetPhysicalDevice());
    init_info.Device = VkDevice(m_deviceManager.GetDevice());
    init_info.QueueFamily = m_deviceManager.GetGraphicsQueueInfo().familyIndex;
    init_info.Queue = VkQueue(m_deviceManager.GetGraphicsQueueInfo().queue);
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = VkDescriptorPool(m_descriptorPool.get());
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = check_vk_result;
    init_info.dispatcher = &m_deviceManager.GetDispatcher();

    if (!ImGui_ImplVulkan_Init(&init_info, VkRenderPass(renderPass)))
        return false;

    // Setup Style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // upload imgui fonts
    {
        // Use any command queue
        VkCommandPool command_pool = VkCommandPool(commandPool);
        VkCommandBuffer command_buffer = VkCommandBuffer(cmdBuffer);

        VkResult err = m_deviceManager.GetDispatcher().vkResetCommandPool(init_info.Device, command_pool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = m_deviceManager.GetDispatcher().vkBeginCommandBuffer(command_buffer, &begin_info);
        check_vk_result(err);

        if (!ImGui_ImplVulkan_CreateFontsTexture(command_buffer))
            return false;

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        err = m_deviceManager.GetDispatcher().vkEndCommandBuffer(command_buffer);
        check_vk_result(err);
        err = m_deviceManager.GetDispatcher().vkQueueSubmit(init_info.Queue, 1, &end_info, VK_NULL_HANDLE);
        check_vk_result(err);

        err = m_deviceManager.GetDispatcher().vkDeviceWaitIdle(init_info.Device);
        check_vk_result(err);
        ImGui_ImplVulkan_InvalidateFontUploadObjects();
    }

    return true;
}

void 
VulkanWindowRenderer::ImGuiPipeline::RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VkCommandBuffer(frameInfo.drawCmdBuffer));
}

bool
VulkanWindowRenderer::OnWindowSizeChanged()
{
    // compute perspective projection
    const float aspectRatio = 
        ((float)GetSwapChainExtend().width) / ((float)GetSwapChainExtend().height);
    const float fov = 60.f;
    Matrix4 projection;
    CommonUtils::GetPerspectiveProjectionMatrixVulkan(aspectRatio, fov, 0.1f, 100.f, projection);

    std::array<float, 16> matrixData;
    projection.GetRaw(matrixData);
    if (!m_graphicsPipeline.UpdateProjectionMatrix(matrixData, m_cmdBuffer.get()))
        return false;

     if (!m_primitivePipeline.UpdateProjectionMatrix(matrixData, m_cmdBuffer.get()))
         return false;

    return true;
}

bool
VulkanWindowRenderer::Draw(float /*dtMsecs*/, SwapChainRenderer::RenderStats& stats)
{
    SwapChainRenderer::RenderStatus status = RenderPipelines(
        { &m_graphicsPipeline, &m_primitivePipeline, &m_imguiPipeline }, stats);

    if (status == SwapChainRenderer::RenderStatus::eRENDER_STATUS_RESIZE)
        OnWindowSizeChanged();
    else if (status != RenderStatus::eRENDER_STATUS_COMPLETE)
        return false;

    return true; 
}

void 
VulkanWindowRenderer::UpdateCamera(const Camera& camera)
{
    Matrix4 view;
    camera.GetViewRenderMatrix(view);

    std::array<float, 16> matrixData;
    view.GetRaw(matrixData);
    m_graphicsPipeline.UpdateViewMatrix(matrixData, m_cmdBuffer.get());
    m_primitivePipeline.UpdateViewMatrix(matrixData, m_cmdBuffer.get());
}

void VulkanWindowRenderer::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    ImGui_ImplVulkan_Shutdown();

    SwapChainRenderer::Clear();
}

bool 
VulkanWindowRenderer::Init()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");

    SwapChainRenderer::InitInfo baseInfo = {};
    if (!SwapChainRenderer::Init(baseInfo))
        return false;

    m_imguiPipeline.SetupPipeline(m_renderPass.get(), m_graphicsCommandPool.get(), m_cmdBuffer.get());

    return true;
}

} // namespace hephaestus