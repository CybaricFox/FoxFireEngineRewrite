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

//Traces allocations
//#ifndef VULKAN_ALLOCATOR_TRACE
//    #define VULKAN_ALLOCATOR_TRACE 1
//#endif

//Enable or disable the use of a custom allocator in Vulkan
#ifndef VULKAN_USE_CUSTOM_ALLOCATOR
    #define VULKAN_USE_CUSTOM_ALLOCATOR 1
#endif

#include "VulkanBackendShader.h"
#include "../IRendererBackend.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "src/modules/engine/Core/GameInstance.h"
#include <sstream>

class VulkanBackend final : public IRendererBackend{
private:
    int majorVersion = 0;
    int minorVersion = 0;
    int patchVersion = 0;

    bool hasFlag(const VulkanBuffer& buffer, VkMemoryPropertyFlagBits flag);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);

    bool createSurface(const Platform& platform);
    bool recreateSwapchain();
    //Semaphore syncs between gpu threads
    //Fence syncs between gpu and application
    bool swapchainAcquireNextImageIndex(unsigned long timeout, VkSemaphore semaphore, VkFence fence, unsigned int& outImageIndex);
    void presentSwapchain();
    void allocateCommandBuffers();
    bool createModule(const VulkanShaderStageConfig &config, VulkanShaderStage &stage) const;
    VkSamplerAddressMode convertTextureRepeatToVulkan(const String &axis, TextureRepeat repeat);
    VkFilter convertTextureFilterToVulkan(const String &op, TextureFilter filter);
    VkFormat convertChannelCountToFormat(unsigned char channelCount, VkFormat defaultFormat);
    bool copyBufferRange(VkBuffer source, ULong sourceOffset, VkBuffer dest, ULong destOffset, ULong size);

#if VULKAN_USE_CUSTOM_ALLOCATOR == 1
    /**
     * @brief Allocation used by vulkan if enabled.
     * @link https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/PFN_vkAllocationFunction.html
     * @param data
     * @param size
     * @param alignment
     * @param scope
     * @return
     */
    static void* vulkanAllocate(void* data, const size_t size, const size_t alignment, VkSystemAllocationScope scope) {
        if (size == 0) return nullptr;
        if (!FF_Memory::isInitialized()) {
            Logger::logFatal("Vulkan attempted an allocation when FF_Memory is not initialized!");
            return nullptr;
        }

        void* result = FF_Memory::ff_allocate(size, RENDER_BACKEND_MANUAL, static_cast<unsigned short>(alignment));

        #ifdef VULKAN_ALLOCATOR_TRACE
        std::ostringstream oss{};
        oss << result;
        Logger::logDebug("Vulkan allocated memory block: " + oss.str() + " Size: " + std::to_string(size) + " Alignment: " + std::to_string(alignment));
        #endif

        return result;
    }

    static void vulkanFree(void* data, void* memory) {
        if (!memory) {
            #ifdef VULKAN_ALLOCATOR_TRACE
            Logger::logDebug("Vulkan cannot free memory because memory is null.");
            #endif
            return;
        }

        ULong size = 0;
        unsigned short alignment = 1;
        const bool result = FF_Memory::getSizeAndAlignment(memory, size, alignment);
        std::ostringstream oss{};
        oss << memory;
        String string{};
        string.setString(oss.str().c_str(), oss.str().length());

        if (!result) {
            Logger::logError("Vulkan failed to free memory block: " + string);
            return;
        }

        #ifdef VULKAN_ALLOCATOR_TRACE
        Logger::logDebug("Vulkan successfully freed memory block: " + oss.str());
        #endif
        FF_Memory::ff_free(memory, size, RENDER_BACKEND_MANUAL, alignment);
    }

    static void* vulkanReallocate(void* data, void* original, const size_t size, const size_t alignment, const VkSystemAllocationScope scope) {
        //No reallocation necessary. Just allocate.
        if (!original) {
            return vulkanAllocate(data, size, alignment, scope);
        }

        std::ostringstream oss{};
        oss << original;
        ULong allocationSize = 0;
        unsigned short allocationAlignment = 1;
        if (!FF_Memory::getSizeAndAlignment(original, allocationSize, allocationAlignment)) {
            String string{};
            string.setString(oss.str().c_str(), oss.str().length());
            Logger::logError("Vulkan cannot realign memory block: " + string);
            return nullptr;
        }
        if (size == 0) {
            FF_Memory::ff_free(original, allocationSize, RENDER_BACKEND_MANUAL, allocationAlignment);
            return nullptr;
        }

        if (allocationAlignment != alignment) {
            Logger::logError("Vulkan reallocation is using a different alignment than the original! Original: " + toString(allocationAlignment) + " Passed: " + toString(alignment));
            return nullptr;
        }

        void* result = vulkanAllocate(data, size, allocationAlignment, scope);
        if (!result) {
        #ifdef VULKAN_ALLOCATOR_TRACE
            Logger::logDebug("Vulkan failed to reallocate original: " + oss.str());
        #endif
            return result;
        }

        #ifdef VULKAN_ALLOCATOR_TRACE
        std::ostringstream rss{};
        rss << result;
        Logger::logDebug("Vulkan successfully reallocated memory block " + oss.str() + " to " + rss.str());
        #endif

        FF_Memory::ff_copy(result, original, size);
        #ifdef VULKAN_ALLOCATOR_TRACE
        Logger::logDebug("Now freeing original memory " + oss.str());
        #endif

        FF_Memory::ff_free(original, allocationSize, RENDER_BACKEND_MANUAL, allocationAlignment);

        return result;
    }

    static void vulkanInternalAllocate(void* data, const size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
        #ifdef VULKAN_ALLOCATOR_TRACE
        Logger::logDebug("Vulkan is externally allocating " + std::to_string(size) + " bytes.");
        #endif

        FF_Memory::reportAllocation(size, RENDER_BACKEND_AUTO);
    }

    static void vulkanInternalFree(void* data, const size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
        #ifdef VULKAN_ALLOCATOR_TRACE
        Logger::logDebug("Vulkan is externally freeing " + std::to_string(size) + " bytes.");
        #endif

        FF_Memory::removeReport(size, RENDER_BACKEND_AUTO);
    }

    bool createVulkanAllocator(VkAllocationCallbacks* callbacks) {
        if (!callbacks) {
            return false;
        }

        callbacks->pfnAllocation = vulkanAllocate;
        callbacks->pfnReallocation = vulkanReallocate;
        callbacks->pfnFree = vulkanFree;
        callbacks->pfnInternalAllocation = vulkanInternalAllocate;
        callbacks->pfnInternalFree = vulkanInternalFree;
        callbacks->pUserData = &vulkanContext;

        return true;
    }
#endif

public:
    VulkanBackend() = default;
    ~VulkanBackend() override;

    static VulkanContext vulkanContext;

    bool initialize(Platform &platform, const RendererBackendConfig &config, unsigned char &outRenderTargetCount, ResourceSystem *resources) override;

    Renderpass* getRenderpass(String name) override;
    Texture* getWindowAttachment(unsigned char index) override;
    Texture* getDepthAttachment() override;
    unsigned char getWindowAttachmentIndex() override;
    bool isMultithreaded() override {return vulkanContext.isMultithreaded();}

    void setVersion(const GameInstance& gameInstance);

    void resize(unsigned short width, unsigned short height) override;
    bool beginFrame(float deltaTime) override;
    bool endFrame(float deltaTime) override;
    void drawGeometry(const GeometryRenderData &data, Material &defaultMaterial) override;
    void createTexture(const unsigned char *pixels, Texture &texture) override;
    void destroyTexture(Texture &texture) override;
    bool createGeometry(Geometry &geometry, unsigned int vertexSize, unsigned int vertexCount, Vertex* vertices, unsigned int indexSize, unsigned int indexCount, void *indices) override;
    void destroyGeometry(Geometry &geometry) override;
    bool beginRenderpass(Renderpass& renderpass, RenderTarget& target) override;
    bool endRenderpass(Renderpass& renderpass) override;
    bool createShader(Shader &shader, ShaderConfig &config, Renderpass &renderpass, unsigned char stageCount, DynamicArray<String> &stageFileNames, DynamicArray<ShaderStage> &stages) override;
    bool initializeShader(Shader &shader) override;
    void destroyShader(Shader &shader) override;
    bool useShader(Shader &shader) override;
    bool bindShaderGlobals(Shader &shader) override;
    void bindShaderInstance(Shader &shader, unsigned instanceId) override;
    bool setUniform(Shader &shader, ShaderUniform &uniform, void *value) override;
    bool applyShaderGlobals(Shader &shader) override;
    bool applyShaderInstance(Shader &shader, bool update) override;
    bool acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, TextureMap **maps) override;
    bool releaseInstanceResources(const Shader &shader, unsigned int instanceId) override;
    bool acquireTextureMapResources(TextureMap &textureMap) override;
    void releaseTextureMapResources(TextureMap &textureMap) override;
    void createWritableTexture(Texture& texture) override;
    void resizeTexture(Texture& texture, unsigned int width, unsigned int height) override;
    void writeTextureData(Texture& texture, unsigned int offset, unsigned int size, const unsigned char* pixels) override;
    void createRenderTarget(unsigned char attachmentCount, DynamicArray<Texture *> &attachments, Renderpass &renderpass, unsigned width, unsigned height, RenderTarget& outTarget) override;
    void destroyRenderTarget(RenderTarget &target, bool freeMemory) override;
    void createRenderpass(Renderpass &outRenderpass, float depth, unsigned stencil, bool hasPreviousPass, bool hasNextPass) override;
    void destroyRenderpass(Renderpass &renderpass) override;

    bool createBuffer(RenderBuffer &buffer) override;
    void destroyBuffer(RenderBuffer &buffer) override;
    bool bindBuffer(RenderBuffer &buffer, ULong offset) override;
    bool unbindBuffer(RenderBuffer &buffer) override;
    void* mapBufferMemory(RenderBuffer &buffer, ULong offset, ULong size) override;
    void unmapBufferMemory(RenderBuffer &buffer, ULong offset, ULong size) override;
    bool flushBuffer(RenderBuffer &buffer, ULong offset, ULong size) override;
    bool readBuffer(RenderBuffer &buffer, ULong offset, ULong size, void *&outMemory) override;
    bool resizeBuffer(RenderBuffer &buffer, ULong newSize) override;
    bool loadBufferRange(RenderBuffer &buffer, ULong offset, ULong size, const void *data) override;
    bool copyBufferRange(RenderBuffer &source, ULong sourceOffset, RenderBuffer &destination, ULong destOffset, ULong size) override;
    bool drawBuffer(RenderBuffer &buffer, ULong offset, unsigned elementCount, bool bindOnly) override;
};
