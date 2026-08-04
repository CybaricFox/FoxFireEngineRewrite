//
// Created by cmorg on 7/30/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanCommandBuffer.h"
#include "src/modules/engine/Core/Engine.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

class VulkanRenderpass {
private:
    VkRenderPass handle{};
    Vector4f renderArea{};
    Vector4f clearColor{};
    float depth = 0;
    unsigned int stencil = 0;
    unsigned char clearFlags = 0;
    bool bHasPreviousPass = false;
    bool bHasNextPass = false;

    EngineRenderpasses id{};
    DynamicArray<VkFramebuffer> framebuffers{};

public:
    VkRenderPass& getHandle() { return handle; }
    EngineRenderpasses getId() const { return id; }
    [[nodiscard]] bool usesDepth() const { return (clearFlags & RENDERPASS_CLEAR_DEPTH) != 0; }
    VkFramebuffer& getFramebuffer(const unsigned int index) { return framebuffers[index]; }

    void setRenderArea(const Vector4f newRenderArea) { renderArea = newRenderArea; }
    void setClearColor(const Vector4f color) { clearColor = color; }
    void setClearFlags(const unsigned char newClearFlags) {clearFlags = newClearFlags;}
    void setPreviousPass(const bool hasPrevious) { bHasPreviousPass = hasPrevious; }
    void setNextPass(const bool hasNext) { bHasNextPass = hasNext; }
    void setWidth(const float width) { renderArea.z = width; }
    void setHeight(const float height) { renderArea.w = height; }
    void setId(const EngineRenderpasses newId) {id = newId;}

    void destroyRenderpass(VulkanDevice &device);
    void beginRenderpass(VulkanCommandBuffer& commandBuffer, VkFramebuffer frameBuffer) const;
    void endRenderpass(VulkanCommandBuffer& commandBuffer);

    void createRenderpass(
        Vector4f render, float newDepth,
        unsigned int newStencil, VkSurfaceFormatKHR &format, VulkanDevice &device);
    void setupFramebuffers(unsigned int count);
    void destroyFramebuffers(VulkanDevice &device);
};
