//
// Created by cmorg on 7/28/2026.
//

#include "VulkanImage.h"

#include "VulkanUtils.h"

void VulkanImage::createImage(VkImageType imageType, const unsigned int newWidth, const unsigned int newHeight, VkFormat format,
                              VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, const bool createView,
                              VkImageAspectFlags aspect, VulkanDevice& device) {

    width = newWidth;
    height = newHeight;

    VkImageCreateInfo imageCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 4;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VulkanUtils::vulkanCheck(vkCreateImage(device.getLogicalDevice(), &imageCreateInfo, nullptr, &handle));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.getLogicalDevice(), handle, &memRequirements);
    const int memoryType = VulkanUtils::findMemoryIndex(static_cast<int>(memRequirements.memoryTypeBits), memoryPropertyFlags, device.getPhysicalDevice());
    if (memoryType == -1) {
        Logger::logError("Memory type could not be found. The image in invalid.");
        return;
    }

    VkMemoryAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = memoryType;
    VulkanUtils::vulkanCheck(vkAllocateMemory(device.getLogicalDevice(), &allocateInfo, nullptr, &deviceMemory));
    VulkanUtils::vulkanCheck(vkBindImageMemory(device.getLogicalDevice(), handle, deviceMemory, 0));

    if (createView) {
        view = nullptr;
        createImageView(format, aspect, device);
    }
}

void VulkanImage::destroy(VulkanDevice &device) {
    if (view) {
        vkDestroyImageView(device.getLogicalDevice(), view, nullptr);
        view = nullptr;
    }
    if (handle) {
        vkDestroyImage(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }
    if (deviceMemory) {
        vkFreeMemory(device.getLogicalDevice(), deviceMemory, nullptr);
        deviceMemory = nullptr;
    }
}

void VulkanImage::transitionImageLayout(const VulkanCommandBuffer &commandBuffer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VulkanDevice& device) const {
    //memory barrier ensures commands called before this use the old layout, and commands after this use the new layout.
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = device.getGraphicsQueueIndex();
    barrier.dstQueueFamilyIndex = device.getGraphicsQueueIndex();
    barrier.image = handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    //We dont care about the old layout, so transition the image to an optimal layout
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    //Transition from transfer destination to shader-readonly layout.
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    //not supported
    else {
        Logger::logFatal("Invalid layout transition!");
        return;
    }

    vkCmdPipelineBarrier(commandBuffer.handle, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr,1, &barrier);
}

void VulkanImage::copyFromBuffer(const VkBuffer buffer, const VulkanCommandBuffer &commandBuffer) const {
    VkBufferImageCopy region;
    FF_Memory::ff_clear(&region, sizeof(VkBufferImageCopy));
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(commandBuffer.handle, buffer, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanImage::createImageView(VkFormat format, VkImageAspectFlags aspectFlags, VulkanDevice& device) {
    VkImageViewCreateInfo viewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCreateInfo.image = handle;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VulkanUtils::vulkanCheck(vkCreateImageView(device.getLogicalDevice(), &viewCreateInfo, nullptr, &view));
}
