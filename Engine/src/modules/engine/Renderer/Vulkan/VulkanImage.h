/**
*   @file VulkanImage.h
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
#include "VulkanDevice.h"
#include "src/modules/engine/Resources/EngineTextureTypes.h"


class VulkanImage {
private:
    VkImage handle{};
    VkDeviceMemory deviceMemory{};
    VkImageView view{};
    unsigned int width = 0;
    unsigned int height = 0;

    void createImageView(VkFormat format, VkImageAspectFlags aspectFlags, VulkanDevice &device, TextureType type);

public:
    void destroy(VulkanDevice& device);

    VkImageView& getImageView() {return view;}
    VkImage& getImage() {return handle;}
    VkDeviceMemory& getMemory() {return deviceMemory;}

    void setImage(VkImage image) {handle = image;}
    void setWidth(const unsigned int newWidth) {width = newWidth;}
    void setHeight(const unsigned int newHeight) {height = newHeight;}


    void transitionImageLayout(VulkanCommandBuffer &commandBuffer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, TextureType type, VulkanDevice
                               &device) const;
    void copyFromBuffer(VkBuffer buffer, VulkanCommandBuffer &commandBuffer, TextureType type) const;

    void createImage(
            TextureType imageType, unsigned int newWidth, unsigned int newHeight, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, bool createView,
            VkImageAspectFlags aspect, VulkanDevice &device
    );
};
