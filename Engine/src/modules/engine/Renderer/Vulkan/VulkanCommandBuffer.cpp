//
// Created by cmorg on 7/30/2026.
//

#include "VulkanCommandBuffer.h"

#include "VulkanUtils.h"

VulkanCommandBuffer VulkanCommandBuffer::allocateAndBeginSingleUseCommandBuffer(VulkanDevice &device) {
    VulkanCommandBuffer buffer{};
    buffer.allocateCommandBuffer(true, device);
    buffer.beginCommandBuffer(true, false, false);
    return buffer;
}

void VulkanCommandBuffer::allocateCommandBuffer(const bool bIsPrimary, VulkanDevice& device) {
    VkCommandBufferAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = device.getCommandPool();
    allocateInfo.level = bIsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = nullptr;

    state = NOT_ALLOCATED;
    VulkanUtils::vulkanCheck(vkAllocateCommandBuffers(device.getLogicalDevice(), &allocateInfo, &handle));
    state = READY;
}

void VulkanCommandBuffer::beginCommandBuffer(const bool bIsSingleUse, const bool bIsRenderpassContinue, const bool bIsConcurrent) {
    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    beginInfo.flags = 0;
    if (bIsSingleUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (bIsRenderpassContinue) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (bIsConcurrent) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VulkanUtils::vulkanCheck(vkBeginCommandBuffer(handle, &beginInfo));
    state = RECORDING;
}

void VulkanCommandBuffer::freeCommandBuffer(VulkanDevice& device) {
    vkFreeCommandBuffers(device.getLogicalDevice(), device.getCommandPool(), 1, &handle);
    handle = nullptr;
    state = NOT_ALLOCATED;
}

void VulkanCommandBuffer::endCommandBuffer() {
    VulkanUtils::vulkanCheck(vkEndCommandBuffer(handle));
    state = RECORDING_ENDED;
}

void VulkanCommandBuffer::updateSubmittedCommandBuffer() {
    state = SUBMITTED;
}

void VulkanCommandBuffer::resetCommandBuffer() {
    state = READY;
}

void VulkanCommandBuffer::endSingleUseCommandBuffer(VkQueue queue, VulkanDevice& device) {
    endCommandBuffer();

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &handle;

    VulkanUtils::vulkanCheck(vkQueueSubmit(queue, 1, &submitInfo, nullptr));

    //Wait for queue to finish since there is no fence here
    VulkanUtils::vulkanCheck(vkQueueWaitIdle(queue));

    freeCommandBuffer(device);
}