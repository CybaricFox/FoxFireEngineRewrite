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
    unsigned long freeListMemoryRequirement = 0;
    void* memoryBlock = nullptr;
    FreeList bufferFreeList{};

    void bindBuffer(VulkanDevice &device, unsigned long offset) const;
    bool resizeBuffer(VulkanDevice &device, unsigned long newSize, VkQueue queue, VkCommandPool pool);
    void destroyFreeList();

public:
    /**
     * @brief Returns the VkBuffer.
     * @return
     */
    VkBuffer& getBuffer() {return handle;}
    bool createBuffer(VulkanDevice& device, unsigned long size, VkBufferUsageFlagBits usage, unsigned int memoryFlags, bool bBind);

    /**
     * @brief Copys the buffer data to another buffer.
     * @param device
     * @param pool
     * @param fence
     * @param queue
     * @param source Source buffer.
     * @param sourceOffset
     * @param dest Destination buffer.
     * @param destOffset
     * @param size
     */
    void copyBufferData(VulkanDevice& device, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, unsigned long sourceOffset, VkBuffer dest, unsigned long destOffset, unsigned long size);

    /**
     * @brief Loads the buffers data to vulkan.
     * @param device
     * @param offset
     * @param size
     * @param data OUT data.
     */
    void loadBufferData(VulkanDevice &device, unsigned long offset, unsigned long size, const void *data) const;
    void destroyBuffer(VulkanDevice &device);

    /**
     * @brief Allocates memory to the buffer's free list.
     * @param size
     * @param outOffset
     * @return False if the allocation fails.
     */
    bool allocate(unsigned long size, unsigned long& outOffset);

    /**
     * @brief Frees memory in the buffers free list.
     * @param size
     * @param offset
     * @return False if the memory cannot be freed.
     */
    [[nodiscard]] bool free(unsigned long size, unsigned long offset);

    /**
     * @brief Unlocks the buffer.
     * @param device
     */
    void unlockBuffer(VulkanDevice &device) const;

    /**
     * @brief Locks the buffer.
     * @param device
     * @param offset
     * @param size
     * @param flags
     * @return Returns the buffers data.
     */
    void* lockBuffer(VulkanDevice &device, unsigned long offset, unsigned long size, unsigned int flags) const;

    /**
     * @brief Locks the buffer with the maximum possible size vulkan allows.
     * @param device
     * @param offset
     * @param flags
     * @return Returns the buffers data.
     */
    void* lockBufferWhole(VulkanDevice &device, unsigned long offset, unsigned int flags) const;
};
