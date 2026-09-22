/**
*   @file VulkanRenderpass.h
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

#include "VulkanCommandBuffer.h"
#include "src/modules/engine/Core/Engine.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Renderer/IRenderpass.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

class VulkanRenderpass : public IRenderpass{
private:
    VkRenderPass handle{};
    float depth = 0;
    unsigned int stencil = 0;
    bool bHasPreviousPass = false;
    bool bHasNextPass = false;

    String name{};
    unsigned int id{};
    DynamicArray<VkFramebuffer> framebuffers{};

public:
    void shutdown();

    VkRenderPass& getHandle() { return handle; }
    [[nodiscard]] unsigned int getId() const { return id; }
    VkFramebuffer& getFramebuffer(const unsigned int index) { return framebuffers[index]; }
    [[nodiscard]] String getName() const { return name; }
    [[nodiscard]] float getDepth() const { return depth; }
    [[nodiscard]] unsigned int getStencil() const { return stencil; }

    void setPreviousPass(const bool hasPrevious) { bHasPreviousPass = hasPrevious; }
    void setNextPass(const bool hasNext) { bHasNextPass = hasNext; }
    void setId(const unsigned int newId) {id = newId;}
    void setName(const String &newName) { name = newName; }
    void setDepth(const float newDepth) { depth = newDepth; }
    void setStencil(const unsigned int newStencil) { stencil = newStencil; }

    void setupFramebuffers(unsigned int count);
    void destroyFramebuffers(VulkanDevice &device);
};
