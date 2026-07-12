//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "../RendererBackend.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "src/modules/engine/Core/GameInstance.h"

class VulkanBackend final : public RendererBackend{
private:
    int majorVersion = 0;
    int minorVersion = 0;
    int patchVersion = 0;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);

    bool createSurface(Platform& platform);
    bool recreateSwapchain();
    //Semaphore syncs between gpu threads
    //Fence syncs between gpu and application
    bool swapchainAcquireNextImageIndex(unsigned long timeout, VkSemaphore semaphore, VkFence fence, unsigned int& outImageIndex);
    void presentSwapchain();
    void createRenderpass(float x, float y, float w, float h, float r, float g, float b, float a, float depth, unsigned int stencil);
    void beginRenderpass(VulkanCommandBuffer& commandBuffer, VkFramebuffer frameBuffer);
    void endRenderpass(VulkanCommandBuffer& commandBuffer);
    void allocateCommandBuffer(bool bIsPrimary, VulkanCommandBuffer& commandBuffer);
    void freeCommandBuffer(VulkanCommandBuffer& commandBuffer);
    void beginCommandBuffer(VulkanCommandBuffer& commandBuffer, bool bIsSingleUse, bool bIsRenderpassContinue, bool bIsConcurrent);
    void endCommandBuffer(VulkanCommandBuffer& commandBuffer);
    void updateSubmittedCommandBuffer(VulkanCommandBuffer& commandBuffer);
    void resetCommandBuffer(VulkanCommandBuffer& commandBuffer);
    void allocateAndBeginSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer);
    void endSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer, VkQueue queue);
    void allocateCommandBuffers();
    void createFence(bool bCreateSignaled, VulkanFence& fence);
    bool waitForFence(VulkanFence& fence, unsigned long timeout);
    void resetFence(VulkanFence& fence);
public:
    VulkanBackend() = default;
    ~VulkanBackend() override;

    static VulkanContext vulkanContext;
    static unsigned int cachedWidth;
    static unsigned int cachedHeight;


    bool initialize(String appName, Platform& platform, unsigned int width, unsigned int height) override;

    void setVersion(const GameInstance& gameInstance);

    void resize(unsigned short width, unsigned short height) override;
    bool beginFrame(float deltaTime) override;
    bool endFrame(float deltaTime) override;
};
