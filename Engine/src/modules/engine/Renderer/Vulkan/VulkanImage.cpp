//
// Created by cmorg on 7/28/2026.
//

#include "VulkanImage.h"

#include "VulkanUtils.h"

void VulkanImage::createImage(const TextureType imageType, const unsigned int newWidth, const unsigned int newHeight, VkFormat format,
                              VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, const bool createView,
                              VkImageAspectFlags aspect, VulkanDevice& device, const VkAllocationCallbacks* allocator) {

    width = newWidth;
    height = newHeight;
    memoryFlags = memoryPropertyFlags;

    VkImageCreateInfo imageCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 4;
    imageCreateInfo.arrayLayers = imageType == TEXTURE_CUBE ? 6 : 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (imageType == TEXTURE_CUBE) {
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    switch (imageType) {
        case TEXTURE_CUBE:
        case TEXTURE_2D: {
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            break;
        }
    }

    VulkanUtils::vulkanCheck(vkCreateImage(device.getLogicalDevice(), &imageCreateInfo, allocator, &handle));

    vkGetImageMemoryRequirements(device.getLogicalDevice(), handle, &memoryRequirements);
    const int memoryType = VulkanUtils::findMemoryIndex(static_cast<int>(memoryRequirements.memoryTypeBits), memoryPropertyFlags, device.getPhysicalDevice());
    if (memoryType == -1) {
        Logger::logError("Memory type could not be found. The image in invalid.");
        return;
    }

    VkMemoryAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryType;
    VulkanUtils::vulkanCheck(vkAllocateMemory(device.getLogicalDevice(), &allocateInfo, allocator, &deviceMemory));
    VulkanUtils::vulkanCheck(vkBindImageMemory(device.getLogicalDevice(), handle, deviceMemory, 0));

    const bool isDeviceMemory = (memoryFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    FF_Memory::reportAllocation(memoryRequirements.size, isDeviceMemory ? GPU : RENDER_BACKEND_MANUAL);

    if (createView) {
        view = nullptr;
        createImageView(format, aspect, device, imageType, allocator);
    }
}

void VulkanImage::destroy(VulkanDevice &device, const VkAllocationCallbacks* allocator) {
    if (view) {
        vkDestroyImageView(device.getLogicalDevice(), view, allocator);
        view = nullptr;
    }
    if (handle) {
        vkDestroyImage(device.getLogicalDevice(), handle, allocator);
        handle = nullptr;
    }
    if (deviceMemory) {
        vkFreeMemory(device.getLogicalDevice(), deviceMemory, allocator);
        deviceMemory = nullptr;
    }

    const bool isDeviceMemory = (memoryFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    FF_Memory::removeReport(memoryRequirements.size, isDeviceMemory ? GPU : RENDER_BACKEND_MANUAL);
    FF_Memory::ff_clear(&memoryRequirements, sizeof(VkMemoryRequirements));
}

void VulkanImage::transitionImageLayout(VulkanCommandBuffer &commandBuffer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, const TextureType type, VulkanDevice& device) const {
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
    barrier.subresourceRange.layerCount = type == TEXTURE_CUBE ? 6 : 1;

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

    vkCmdPipelineBarrier(commandBuffer.getHandle(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr,1, &barrier);
}

void VulkanImage::copyFromBuffer(VkBuffer buffer, VulkanCommandBuffer &commandBuffer, const TextureType type) const {
    VkBufferImageCopy region{};
    FF_Memory::ff_clear(&region, sizeof(VkBufferImageCopy));
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = type == TEXTURE_CUBE ? 6 : 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(commandBuffer.getHandle(), buffer, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanImage::createImageView(VkFormat format, VkImageAspectFlags aspectFlags, VulkanDevice& device, const TextureType type, const VkAllocationCallbacks* allocator) {
    VkImageViewCreateInfo viewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCreateInfo.image = handle;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = type == TEXTURE_CUBE ? 6 : 1;
    switch (type) {
        case TEXTURE_2D: {
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        }
        case TEXTURE_CUBE: {
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        }
    }

    VulkanUtils::vulkanCheck(vkCreateImageView(device.getLogicalDevice(), &viewCreateInfo, allocator, &view));
}
