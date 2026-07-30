//
// Created by cmorg on 7/30/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanState.h"
#include "VulkanCommandBuffer.h"


class VulkanRenderpass {
private:
    VkRenderPass handle{};
    float x = 0;
    float y = 0;
    float w = 0;
    float h = 0;
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;
    float depth = 0;
    unsigned int stencil = 0;
    VulkanState state{};

public:
    VkRenderPass& getHandle() { return handle; }

    void setWidth(const float width) {w = width;}
    void setHeight(const float height) {h = height;}
    void setX(const float newX) {x = newX;}
    void setY(const float newY) {y = newY;}

    void destroyRenderpass(VulkanDevice &device);
    void beginRenderpass(VulkanCommandBuffer& commandBuffer, VkFramebuffer frameBuffer);
    void endRenderpass(VulkanCommandBuffer& commandBuffer);

    void createRenderpass(float newX, float newY, float newW, float newH, float newR, float newG,
                                      float newB,
                                      float newA, float newDepth, unsigned int newStencil, VkSurfaceFormatKHR &format,
                                      VulkanDevice &device);
};
