//
// Created by cmorg on 7/30/2026.
//

#include "VulkanBuffer.h"

#include "VulkanCommandBuffer.h"
#include "VulkanUtils.h"

bool VulkanBuffer::createBuffer(VulkanDevice& device, const unsigned long size, VkBufferUsageFlagBits usage, const unsigned int memoryFlags, const bool bBind) {
    totalSize = size;
    usageFlags = usage;
    memoryPropertyFlags = memoryFlags;

    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VulkanUtils::vulkanCheck(vkCreateBuffer(device.getLogicalDevice(), &bufferInfo, nullptr, &handle));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.getLogicalDevice(), handle, &memRequirements);
    memoryIndex = VulkanUtils::findMemoryIndex(static_cast<int>(memRequirements.memoryTypeBits), memoryPropertyFlags, device.getPhysicalDevice());
    if (memoryIndex == -1) {
        Logger::logError("Can't find memory index for vulkan buffer!");
        return false;
    }

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryIndex;

    VkResult result = vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &deviceMemory);
    if (result != VK_SUCCESS) {
        Logger::logError("Failed to allocate memory for a new vulkan buffer: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    if (bBind) {
        bindBuffer(device, 0);
    }

    return true;
}

void VulkanBuffer::bindBuffer(VulkanDevice& device, const unsigned long offset) const {
    VulkanUtils::vulkanCheck(vkBindBufferMemory(device.getLogicalDevice(), handle, deviceMemory, offset));
}

void VulkanBuffer::destroyBuffer(VulkanDevice& device) {
    if (deviceMemory) {
        vkFreeMemory(device.getLogicalDevice(), deviceMemory, nullptr);
        deviceMemory = nullptr;
    }
    if (handle) {
        vkDestroyBuffer(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }
    totalSize = 0;
    bIsLocked = false;
}

bool VulkanBuffer::resizeBuffer(VulkanDevice& device, const unsigned long newSize, VkQueue queue, VkCommandPool pool) {
    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = newSize;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer newBuffer;
    VulkanUtils::vulkanCheck(vkCreateBuffer(device.getLogicalDevice(), &bufferInfo, nullptr, &newBuffer));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device.getLogicalDevice(), newBuffer, &requirements);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = memoryIndex;

    VkDeviceMemory newMemory;
    VkResult result = vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &newMemory);
    if (result != VK_SUCCESS) {
        Logger::logError("Failed to resize vulkan buffer: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    VulkanUtils::vulkanCheck(vkBindBufferMemory(device.getLogicalDevice(), newBuffer, newMemory, 0));

    copyBufferData(device, pool, nullptr, queue, handle, 0, newBuffer, 0, totalSize);

    vkDeviceWaitIdle(device.getLogicalDevice());

    if (deviceMemory) {
        vkFreeMemory(device.getLogicalDevice(), deviceMemory, nullptr);
        deviceMemory = nullptr;
    }
    if (handle) {
        vkDestroyBuffer(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }
    totalSize = newSize;
    deviceMemory = newMemory;
    handle = newBuffer;

    return true;
}

void VulkanBuffer::copyBufferData(VulkanDevice& device, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, unsigned long sourceOffset, VkBuffer dest, unsigned long destOffset, unsigned long size) {
    vkQueueWaitIdle(queue);

    //Create a one time use command buffer
    VulkanCommandBuffer tempBuffer = VulkanCommandBuffer::allocateAndBeginSingleUseCommandBuffer(device);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = sourceOffset;
    copyRegion.dstOffset = destOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(tempBuffer.getHandle(), source, dest, 1, &copyRegion);

    tempBuffer.endSingleUseCommandBuffer(queue, device);
}

void * VulkanBuffer::lockBuffer(VulkanDevice& device, const unsigned long offset, const unsigned long size, const unsigned int flags) const {
    void* data;
    VulkanUtils::vulkanCheck(vkMapMemory(device.getLogicalDevice(), deviceMemory, offset, size, flags, &data));
    return data;
}

void VulkanBuffer::unlockBuffer(VulkanDevice &device) const {
    vkUnmapMemory(device.getLogicalDevice(), deviceMemory);
}

void VulkanBuffer::loadBufferData(VulkanDevice &device, const unsigned long offset, const unsigned long size, const void *data) const {
    void* dataPtr;
    VulkanUtils::vulkanCheck(vkMapMemory(device.getLogicalDevice(), deviceMemory, offset, size, 0, &dataPtr));
    FF_Memory::ff_copy(dataPtr, data, size);
    vkUnmapMemory(device.getLogicalDevice(), deviceMemory);
}
