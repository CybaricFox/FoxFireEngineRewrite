//
// Created by cmorg on 7/10/2026.
//

#pragma once

#include <vulkan/vulkan.h>

#include "VulkanImage.h"
#include "VulkanRenderpass.h"
#include "VulkanState.h"
#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Renderer/Vulkan/VulkanDevice.h"

struct VulkanFramebuffer {
        VkFramebuffer handle;
        unsigned int attachmentCount;
        VkImageView* attachments;
        VulkanRenderpass* renderpass;
};

class VulkanSwapchain {
private:
        VkSurfaceFormatKHR imageFormat{};
        unsigned char maxFramesInFlight = 0;
        VkSwapchainKHR handle{};
        unsigned int imageCount = 0;
        VkImage* images = nullptr;
        VkImageView* imageViews = nullptr;
        VulkanImage depthAttachment{};
        VulkanFramebuffer* framebuffers = nullptr;
        bool bRecreateSwapchain = false;
        bool bIsSwapchainDirty = false;
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