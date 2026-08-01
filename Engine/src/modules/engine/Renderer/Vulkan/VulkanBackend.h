//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "../RendererBackend.h"

#include "VulkanContext.h"
#include "VulkanShader.h"
#include "VulkanUtils.h"
#include "src/modules/engine/Core/GameInstance.h"

struct VulkanTextureData {
    VulkanImage image{};
    VkSampler sampler{};
};

class VulkanBackend final : public RendererBackend{
private:
    int majorVersion = 0;
    int minorVersion = 0;
    int patchVersion = 0;

    VulkanShader vulkanShader{};

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
    void allocateCommandBuffers();
    void uploadRangeOfData(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer &buffer, unsigned long offset, unsigned long size, const
                           void *data);
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
    void updateGlobalState(Mat4 projection, Mat4 view, Vector3f viewPosition, Vector4f ambientColor, int mode) override;
    void updateEntity(const GeometryRenderData &data, Texture &defaultTexture) override;
    void createTexture(const unsigned char *pixels, Texture &texture) override;
    void destroyTexture(Texture &texture) override;
    bool createMaterial(Material &material) override;
    void destroyMaterial(Material &material) override;
};
