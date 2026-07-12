//
// Created by cmorg on 7/10/2026.
//

#pragma once

#include <vulkan/vulkan.h>

#include "VulkanState.h"
#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Renderer/Vulkan/VulkanDevice.h"

struct VulkanRenderpass {
        VkRenderPass handle;
        float x;
        float y;
        float w;
        float h;
        float r;
        float g;
        float b;
        float a;
        float depth;
        unsigned int stencil;
        VulkanState state;
};

struct VulkanFramebuffer {
        VkFramebuffer handle;
        unsigned int attachmentCount;
        VkImageView* attachments;
        VulkanRenderpass* renderpass;
};

struct VulkanImage {
        VkImage handle;
        VkDeviceMemory deviceMemory;
        VkImageView view;
        unsigned int width;
        unsigned int height;
};

class VulkanSwapchain {
private:
        VkSurfaceFormatKHR imageFormat;
        unsigned char maxFramesInFlight;
        VkSwapchainKHR handle;
        unsigned int imageCount;
        VkImage* images;
        VkImageView* imageViews;
        VulkanImage depthAttachment;
        VulkanFramebuffer* framebuffers;
        bool bRecreateSwapchain = false;
        bool bIsSwapchainDirty = false;

        void createImageView(VkFormat format, VulkanImage &image, VkImageAspectFlags aspectFlags, VulkanDevice &device);
        [[nodiscard]] int findMemoryIndex(int typeFilter, unsigned int propertyFlags, VulkanDevice &device);

        void createImage(
                VkImageType imageType, unsigned int width, unsigned int height, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, bool createView,
                VkImageAspectFlags aspect, VulkanImage &outImage, VulkanDevice &device
        );
public:
        [[nodiscard]] VulkanFramebuffer& getFramebuffer(const unsigned int index) const {return framebuffers[index];}
        unsigned int& getImageCount() { return imageCount; }
        unsigned char& getMaxFramesInFlight() { return maxFramesInFlight; }
        VkSurfaceFormatKHR& getImageFormat() { return imageFormat; }
        VkSwapchainKHR& getSwapchain() { return handle; }
        [[nodiscard]] VkImage* getImages() const {return images;}
        [[nodiscard]] bool isRecreatingSwapchain() const {return bRecreateSwapchain;}
        [[nodiscard]] bool needsResize() const {return bIsSwapchainDirty;}

        void enableRecreateSwapchain() {bRecreateSwapchain = true;}
        void finishResize() {bIsSwapchainDirty = false;}
        void finishRecreateSwapchain() {bRecreateSwapchain = false;}
        void resize() {bIsSwapchainDirty = true;}

        bool createSwapchain(unsigned int frameBufferWidth, unsigned int frameBufferHeight, VulkanDevice& device, const VkSurfaceKHR& surface, unsigned int& currentFrame);
        void destroyFramebuffer(unsigned int index, VulkanDevice &device);
        bool detectDepthFormat(VulkanDevice &device);
        void createFramebuffer(unsigned int width, unsigned int height, unsigned int attachmentCount, const VkImageView *view, VulkanFramebuffer &framebuffer, VulkanRenderpass &renderpass, VulkanDevice &device);
        void regenerateFramebuffers(unsigned int frameBufferWidth, unsigned int frameBufferHeight, VulkanRenderpass& renderpass, VulkanDevice& device);
        void destroySwapchain(VulkanDevice &device);
        void createFramebuffers();
};