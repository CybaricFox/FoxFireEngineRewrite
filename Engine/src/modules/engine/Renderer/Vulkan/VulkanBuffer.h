/**
*   @file VulkanBuffer.h
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

#include "vulkan/vulkan.h"
#include "src/modules/engine/Renderer/RenderBuffer.h"

struct VulkanBuffer : IRenderBuffer {
    VkBuffer handle{};
    VkBufferUsageFlags usageFlags{};
    bool bIsLocked = false;
    VkDeviceMemory deviceMemory{};
    VkMemoryRequirements memoryRequirements{};
    int memoryIndex = 0;
    unsigned int memoryPropertyFlags = 0;
};
