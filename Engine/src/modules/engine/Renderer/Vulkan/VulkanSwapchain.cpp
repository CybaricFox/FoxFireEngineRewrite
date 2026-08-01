//
// Created by cmorg on 7/10/2026.
//

#include "VulkanSwapchain.h"

#include <algorithm>

#include "VulkanBackend.h"
#include "VulkanUtils.h"
#include "src/modules/engine/Library/Logger.h"

bool VulkanSwapchain::createSwapchain(const unsigned int frameBufferWidth, const unsigned int frameBufferHeight, VulkanDevice& device, const VkSurfaceKHR& surface, unsigned int& currentFrame) {
    VkExtent2D swapchainExtent{frameBufferWidth, frameBufferHeight};

    bool found = false;
    for (unsigned int i = 0; i < device.getSwapChainSupportInfo().formatCount; i++) {
        VkSurfaceFormatKHR surfaceFormat = device.getSwapChainSupportInfo().formats[i];
        //This format is preferred
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            imageFormat = surfaceFormat;
            found = true;
            break;
        }
    }

    //If not found, just use the first one
    if (!found) {
        imageFormat = device.getSwapChainSupportInfo().formats[0];
    }

    //FIFO is gurarnteed. Use it as the default.
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (unsigned int i = 0; i < device.getSwapChainSupportInfo().presentCount; i++) {
        //Check for mailbox if available. This one is better.
        VkPresentModeKHR mode = device.getSwapChainSupportInfo().presentModes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = mode;
            break;
        }
    }

    device.querySwapChainSupport(device.getPhysicalDevice(), surface, device.getSwapChainSupportInfo());

    if (device.getSwapChainSupportInfo().capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent = device.getSwapChainSupportInfo().capabilities.currentExtent;
    }

    VkExtent2D min = device.getSwapChainSupportInfo().capabilities.minImageExtent;
    VkExtent2D max = device.getSwapChainSupportInfo().capabilities.maxImageExtent;
    swapchainExtent.width = std::clamp(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = std::clamp(swapchainExtent.height, min.height, max.height);

    unsigned int _imageCount = device.getSwapChainSupportInfo().capabilities.minImageCount + 1;
    if (device.getSwapChainSupportInfo().capabilities.maxImageCount > 0 && _imageCount > device.getSwapChainSupportInfo().capabilities.maxImageCount) {
        _imageCount = device.getSwapChainSupportInfo().capabilities.maxImageCount;
    }

    maxFramesInFlight = _imageCount - 1;

    VkSwapchainCreateInfoKHR swapChainCreateInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = _imageCount;
    swapChainCreateInfo.imageFormat = imageFormat.format;
    swapChainCreateInfo.imageColorSpace = imageFormat.colorSpace;
    swapChainCreateInfo.imageExtent = swapchainExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const unsigned int queueFamilyIndices[] = {static_cast<unsigned int>(device.getGraphicsQueueIndex()), static_cast<unsigned int>(device.getPresentQueueIndex())};

    if (device.getGraphicsQueueIndex() != device.getPresentQueueIndex()) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainCreateInfo.preTransform = device.getSwapChainSupportInfo().capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = nullptr;

    if (!VulkanUtils::vulkanCheck(vkCreateSwapchainKHR(device.getLogicalDevice(), &swapChainCreateInfo, nullptr, &handle))) return false;

    currentFrame = 0;
    imageCount = 0;
    if (!VulkanUtils::vulkanCheck(vkGetSwapchainImagesKHR(device.getLogicalDevice(), handle, &imageCount, nullptr))) return false;

    if (!images) {
        images = static_cast<VkImage *>(FF_Memory::ff_allocate(sizeof(VkImage) * imageCount, RENDER));
    }
    if (!imageViews) {
        imageViews = static_cast<VkImageView*>(FF_Memory::ff_allocate(sizeof(VkImageView) * imageCount, RENDER));
    }

    if (!VulkanUtils::vulkanCheck(vkGetSwapchainImagesKHR(device.getLogicalDevice(), handle, &imageCount, images))) return false;

    for (unsigned int i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewCreateInfo.image = images[i];
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = imageFormat.format;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        if (!VulkanUtils::vulkanCheck(vkCreateImageView(device.getLogicalDevice(), &viewCreateInfo, nullptr, &imageViews[i]))) return false;
    }

    if (!detectDepthFormat(device)) {
        device.getDepthFormat() = VK_FORMAT_UNDEFINED;
        Logger::logFatal("Failed to find a supported format!");
        return false;
    }

    depthAttachment.createImage(VK_IMAGE_TYPE_2D,
                swapchainExtent.width,
                swapchainExtent.height,
                device.getDepthFormat(),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                true,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                device
    );

    Logger::logInfo("Successfully created swapchain!");
    return true;
}

bool VulkanSwapchain::detectDepthFormat(VulkanDevice& device) {
    constexpr unsigned long candidateCount = 3;
    VkFormat candidates[candidateCount]{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    constexpr unsigned int flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (VkFormat format : candidates) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), format, &formatProperties);

        if ((formatProperties.linearTilingFeatures & flags) == flags || (formatProperties.optimalTilingFeatures & flags) == flags) {
            device.getDepthFormat() = format;
            return true;
        }
    }

    return false;
}

void VulkanSwapchain::destroyFramebuffer(const unsigned int index, VulkanDevice& device) {
    if (framebuffers[index].handle) {
        vkDestroyFramebuffer(device.getLogicalDevice(), framebuffers[index].handle, nullptr);
        framebuffers[index].handle = nullptr;
    }

    if (framebuffers[index].attachments) {
        FF_Memory::ff_free(framebuffers[index].attachments, sizeof(VkImageView) * framebuffers[index].attachmentCount, RENDER);
        framebuffers[index].attachments = nullptr;
    }

    framebuffers[index].attachmentCount = 0;
    framebuffers[index].renderpass = nullptr;
}

void VulkanSwapchain::regenerateFramebuffers(const unsigned int frameBufferWidth, const unsigned int frameBufferHeight, VulkanRenderpass& renderpass, VulkanDevice& device) {
    for (unsigned int i = 0; i < imageCount; i++) {
        constexpr unsigned int attachmentCount = 2;
        const VkImageView attachments[]{imageViews[i], depthAttachment.getImageView()};

        createFramebuffer(frameBufferWidth, frameBufferHeight, attachmentCount, attachments, framebuffers[i], renderpass, device);
    }
}

void VulkanSwapchain::createFramebuffer(const unsigned int width, const unsigned int height, const unsigned int attachmentCount, const VkImageView* view, VulkanFramebuffer& framebuffer, VulkanRenderpass& renderpass, VulkanDevice& device) {
    framebuffer.attachments = static_cast<VkImageView*>(FF_Memory::ff_allocate(sizeof(VkImageView) * attachmentCount, RENDER));
    for (unsigned int i = 0; i < attachmentCount; i++) {
        framebuffer.attachments[i] = view[i];
    }

    framebuffer.renderpass = &renderpass;
    framebuffer.attachmentCount = attachmentCount;

    VkFramebufferCreateInfo frameBufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    frameBufferCreateInfo.renderPass = renderpass.getHandle();
    frameBufferCreateInfo.attachmentCount = attachmentCount;
    frameBufferCreateInfo.pAttachments = framebuffer.attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    VulkanUtils::vulkanCheck(vkCreateFramebuffer(device.getLogicalDevice(), &frameBufferCreateInfo, nullptr, &framebuffer.handle));
}

void VulkanSwapchain::destroySwapchain(VulkanDevice& device) {
    vkDeviceWaitIdle(device.getLogicalDevice());

    depthAttachment.destroy(device);

    if (imageViews) {
        for (unsigned int i = 0; i < imageCount; i++) {
            if (imageViews[i]) {
                vkDestroyImageView(device.getLogicalDevice(), imageViews[i], nullptr);
                imageViews[i] = nullptr;
            }
        }

        FF_Memory::ff_free(imageViews, sizeof(VkImageView) * imageCount, RENDER);
        imageViews = nullptr;
    }

    if (images) {
        FF_Memory::ff_free(images, sizeof(VkImage) * imageCount, RENDER);
        images = nullptr;
    }

    if (handle) {
        vkDestroySwapchainKHR(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }

    imageCount = 0;
}

void VulkanSwapchain::createFramebuffers() {
    framebuffers = static_cast<VulkanFramebuffer *>(FF_Memory::ff_allocate(sizeof(VulkanFramebuffer) * imageCount, ARRAY));
}
