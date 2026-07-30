//
// Created by cmorg on 7/28/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"


class VulkanImage {
private:
    VkImage handle{};
    VkDeviceMemory deviceMemory{};
    VkImageView view{};
    unsigned int width = 0;
    unsigned int height = 0;

    void createImageView(VkFormat format, VkImageAspectFlags aspectFlags, VulkanDevice &device);

public:
    void destroy(VulkanDevice& device);

    VkImageView& getImageView() {return view;}
    VkImage& getImage() {return handle;}
    VkDeviceMemory& getMemory() {return deviceMemory;}

    void transitionImageLayout(VulkanCommandBuffer &commandBuffer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VulkanDevice &
                               device) const;
    void copyFromBuffer(VkBuffer buffer, VulkanCommandBuffer &commandBuffer) const;

    void createImage(
            VkImageType imageType, unsigned int newWidth, unsigned int newHeight, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, bool createView,
            VkImageAspectFlags aspect, VulkanDevice &device
    );
};
