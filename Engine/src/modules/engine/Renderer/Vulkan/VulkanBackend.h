/**
*   @file VulkanBackend.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "VulkanBackendShader.h"
#include "../RendererBackend.h"

#include "VulkanContext.h"
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
    bool uploadRangeOfData(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer &buffer, unsigned long &outOffset, unsigned long size, const void *data);
    bool freeRangeOfData(VulkanBuffer &buffer, unsigned long offset, unsigned long size);
    bool createBuffers();
    bool createModule(const VulkanShaderStageConfig &config, VulkanShaderStage &stage) const;
public:
    VulkanBackend() = default;
    ~VulkanBackend() override;

    static VulkanContext vulkanContext;
    static unsigned int cachedWidth;
    static unsigned int cachedHeight;

    bool initialize(String appName, Platform &platform, unsigned int width, unsigned int height, ResourceSystem* resources) override;

    void setVersion(const GameInstance& gameInstance);

    void resize(unsigned short width, unsigned short height) override;
    bool beginFrame(float deltaTime) override;
    bool endFrame(float deltaTime) override;
    void drawGeometry(const GeometryRenderData &data, Texture &defaultTexture, Material &defaultMaterial) override;
    void createTexture(const unsigned char *pixels, Texture &texture) override;
    void destroyTexture(Texture &texture) override;
    bool createGeometry(Geometry& geometry, unsigned int vertexSize, unsigned int vertexCount, void* vertices, unsigned int indexSize, unsigned int indexCount, void* indices) override;
    void destroyGeometry(Geometry &geometry) override;
    void createRenderpass(RenderpassProfile profile) override;
    bool beginRenderpass(unsigned char renderpassId) override;
    bool endRenderpass(unsigned char renderpassId) override;
    bool createShader(Shader& shader, unsigned char renderpassId, unsigned char stageCount, DynamicArray<String>& stageFileNames, DynamicArray<ShaderStage>& stages) override;
    bool initializeShader(Shader &shader) override;
    void destroyShader(Shader &shader) override;
    bool useShader(Shader &shader) override;
    bool bindShaderGlobals(Shader &shader) override;
    void bindShaderInstance(Shader &shader, unsigned instanceId) override;
    bool setUniform(Shader &shader, ShaderUniform &uniform, void *value) override;
    bool applyShaderGlobals(Shader &shader) override;
    bool applyShaderInstance(Shader &shader) override;
    bool acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, Texture &defaultTexture) override;
    bool releaseInstanceResources(const Shader &shader, unsigned int instanceId) override;

    bool getRenderpassId(String name, unsigned char &outId) override;
};
