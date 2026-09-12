/**
*   @file VulkanSwapchain.h
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

#include <vulkan/vulkan.h>

#include "VulkanImage.h"
#include "VulkanRenderpass.h"
#include "VulkanState.h"
#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Renderer/Vulkan/VulkanDevice.h"

class VulkanSwapchain {
private:
    VkSurfaceFormatKHR imageFormat{};
    unsigned char maxFramesInFlight = 0;
    VkSwapchainKHR handle{};
    unsigned int imageCount = 0;
    DynamicArray<Texture*> textures{};
    Texture* depthTexture = nullptr;
    bool bRecreateSwapchain = false;
    bool bIsSwapchainDirty = false;
    RenderTarget renderTargets[3]{};

public:
    unsigned int& getImageCount() { return imageCount; }
    unsigned char& getMaxFramesInFlight() { return maxFramesInFlight; }
    VkSurfaceFormatKHR& getImageFormat() { return imageFormat; }
    VkSwapchainKHR& getSwapchain() { return handle; }
    [[nodiscard]] bool isRecreatingSwapchain() const {return bRecreateSwapchain;}
    [[nodiscard]] bool needsResize() const {return bIsSwapchainDirty;}
    RenderTarget& getRenderTarget(const unsigned int index) {return renderTargets[index];}
    Texture* getTexture(const unsigned int index) {return textures[index];}
    [[nodiscard]] Texture* getDepthTexture() const {return depthTexture;}

    void enableRecreateSwapchain() {bRecreateSwapchain = true;}
    void finishResize() {bIsSwapchainDirty = false;}
    void finishRecreateSwapchain() {bRecreateSwapchain = false;}
    void resize() {bIsSwapchainDirty = true;}

    bool createSwapchain(unsigned int frameBufferWidth, unsigned int frameBufferHeight, VulkanDevice &device, const VkSurfaceKHR &surface, unsigned int &currentFrame, IRendererBackend *backendRef);
    bool detectDepthFormat(VulkanDevice &device);
    void destroySwapchain(VulkanDevice &device);
};