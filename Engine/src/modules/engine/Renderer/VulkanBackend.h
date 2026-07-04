//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "RendererBackend.h"
#include "src/modules/engine/Library/Logger.h"

#include "VulkanContext.h"
#include "src/modules/engine/Core/GameInstance.h"

struct VulkanPhysicalDeviceFamilyInfo {
    unsigned int graphicsFamily;
    unsigned int presentFamily;
    unsigned int computeFamily;
    unsigned int transferFamily;
};

struct PhysicalDeviceRequirements {
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    std::vector<const char*> extensionNames{};
    bool samplerAnisotrophy;
    bool discreteGPU;
};

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

    bool createDevice();
    bool createSurface(Platform* platform);
    void createSwapchain(unsigned int width, unsigned int height);
    void recreateSwapchain(unsigned int width, unsigned int height);
    bool selectPhysicalDevice();
    //Semaphore syncs between gpu threads
    //Fence syncs between gpu and application
    bool swapchainAcquireNextImageIndex(unsigned long timeout, VkSemaphore semaphore, VkFence fence, unsigned int* outImageIndex);
    void presentSwapchain(VkSemaphore semaphore, unsigned int presentImageIndex);
    bool detectDepthFormat();
    void querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VulkanSwapChainSupportInfo* swapChainSupportInfo);
    void createImageView(VkFormat format, VulkanImage* image, VkImageAspectFlags aspectFlags);
    void destroySwapchain();
    void createRenderpass(float x, float y, float w, float h, float r, float g, float b, float a, float depth, unsigned int stencil);
    void beginRenderpass(VulkanCommandBuffer* commandBuffer, VkFramebuffer frameBuffer);
    void endRenderpass(VulkanCommandBuffer* commandBuffer);
    void allocateCommandBuffer(bool bIsPrimary, VulkanCommandBuffer* commandBuffer);
    void freeCommandBuffer(VulkanCommandBuffer* commandBuffer);
    void beginCommandBuffer(VulkanCommandBuffer* commandBuffer, bool bIsSingleUse, bool bIsRenderpassContinue, bool bIsConcurrent);
    void endCommandBuffer(VulkanCommandBuffer* commandBuffer);
    void updateSubmittedCommandBuffer(VulkanCommandBuffer* commandBuffer);
    void resetCommandBuffer(VulkanCommandBuffer* commandBuffer);
    void allocateAndBeginSingleUseCommandBuffer(VulkanCommandBuffer* commandBuffer);
    void endSingleUseCommandBuffer(VulkanCommandBuffer* commandBuffer, VkQueue queue);
    void allocateCommandBuffers();

    bool physicalDeviceMeetsRequirements(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        const VkPhysicalDeviceProperties *deviceProperties,
        const VkPhysicalDeviceFeatures *deviceFeatures,
        const PhysicalDeviceRequirements *requirements,
        VulkanPhysicalDeviceFamilyInfo *physicalDeviceFamilyInfo,
        VulkanSwapChainSupportInfo *swapChainSupport);

    void createImage(
        VkImageType imageType,
        unsigned int width,
        unsigned int height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memoryPropertyFlags,
        bool createView,
        VkImageAspectFlags aspect,
        VulkanImage* outImage
        );

public:
    ~VulkanBackend() override;

    static VulkanContext vulkanContext;

    static void vulkanCheck(VkResult result);

    bool initialize(String appName, Platform *platform) override;

    void setVersion(const GameInstance* gameInstance);
};
