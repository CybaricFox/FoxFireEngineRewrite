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
#include "VulkanDevice.h"

class VulkanBuffer {
private:
    unsigned long totalSize = 0;
    VkBuffer handle{};
    VkBufferUsageFlagBits usageFlags{};
    bool bIsLocked = false;
    VkDeviceMemory deviceMemory{};
    int memoryIndex = 0;
    unsigned int memoryPropertyFlags = 0;

    void bindBuffer(VulkanDevice &device, unsigned long offset) const;
    bool resizeBuffer(VulkanDevice &device, unsigned long newSize, VkQueue queue, VkCommandPool pool);
    void* lockBuffer(VulkanDevice &device, unsigned long offset, unsigned long size, unsigned int flags) const;
    void unlockBuffer(VulkanDevice &device) const;

public:
    VkBuffer& getBuffer() {return handle;}

    bool createBuffer(VulkanDevice& device, unsigned long size, VkBufferUsageFlagBits usage, unsigned int memoryFlags, bool bBind);
    void copyBufferData(VulkanDevice& device, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, unsigned long sourceOffset, VkBuffer dest, unsigned long destOffset, unsigned long size);
    void loadBufferData(VulkanDevice &device, unsigned long offset, unsigned long size, const void *data) const;
    void destroyBuffer(VulkanDevice &device);
};
