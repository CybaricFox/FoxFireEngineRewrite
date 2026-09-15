//
// Created by cmorg on 7/10/2026.
//

#include "VulkanSwapchain.h"

#include <algorithm>

#include "VulkanBackend.h"
#include "VulkanUtils.h"
#include "src/modules/engine/Library/Logger.h"
#include "src/modules/engine/Renderer/TextureUtils.h"

bool VulkanSwapchain::createSwapchain(const unsigned int frameBufferWidth, const unsigned int frameBufferHeight, VulkanDevice& device, const VkSurfaceKHR& surface, unsigned int& currentFrame, IRendererBackend* backendRef) {
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

    if (textures.isEmpty()) {
        textures.initialize(imageCount);

        for (unsigned int i = 0; i < imageCount; i++) {
            textures.emplace();

            void* data = FF_Memory::ff_allocate_class<VulkanImage>(sizeof(VulkanImage), TEXTURE);
            String textureName = "Vulkan_Swapchain_Image_0" + std::to_string(i);

            textures[i] = TextureUtils::wrapTexture(textureName, swapchainExtent.width, swapchainExtent.height, 4, false, true, false, data);

            if (!textures[i]) {
                Logger::logFatal("Failed to generate a new swapchain image because texture is null!");
                return false;
            }
        }
    } else {
        for (unsigned int i = 0; i < imageCount; i++) {
            TextureUtils::resizeTexture(*textures[i], swapchainExtent.width, swapchainExtent.height, false, backendRef);
        }
    }

    VkImage images[32]{};
    if (!VulkanUtils::vulkanCheck(vkGetSwapchainImagesKHR(device.getLogicalDevice(), handle, &imageCount, images))) return false;

    for (unsigned int i = 0; i < imageCount; i++) {
        VulkanImage& image = *static_cast<VulkanImage *>(textures[i]->data);
        image.setImage(images[i]);
        image.setWidth(swapchainExtent.width);
        image.setHeight(swapchainExtent.height);
    }

    for (unsigned int i = 0; i < imageCount; i++) {
        VulkanImage& image = *static_cast<VulkanImage *>(textures[i]->data);
        VkImageViewCreateInfo viewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewCreateInfo.image = image.getImage();
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = imageFormat.format;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        if (!VulkanUtils::vulkanCheck(vkCreateImageView(device.getLogicalDevice(), &viewCreateInfo, nullptr, &image.getImageView()))) return false;
    }

    if (!detectDepthFormat(device)) {
        device.getDepthFormat() = VK_FORMAT_UNDEFINED;
        Logger::logFatal("Failed to find a supported format!");
        return false;
    }

    auto image = FF_Memory::ff_allocate_class<VulkanImage>(sizeof(VulkanImage), TEXTURE);
    image->createImage(TEXTURE_2D,
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

    depthTexture = TextureUtils::wrapTexture("Fox_Fire_Default_Depth_Texture", swapchainExtent.width, swapchainExtent.height, device.getChannelCount(), false, true, false, image);

    Logger::logInfo("Successfully created swapchain!");
    return true;
}

bool VulkanSwapchain::detectDepthFormat(VulkanDevice& device) {
    constexpr unsigned long candidateCount = 3;
    VkFormat candidates[candidateCount]{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    constexpr unsigned char sizes[3] = {4, 4, 3};

    constexpr unsigned int flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (unsigned long i = 0; i < candidateCount; i++) {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags) {
            device.setDepthFormat(candidates[i]);
            device.setChannelCount(sizes[i]);
            return true;
        }
        if ((properties.optimalTilingFeatures & flags) == flags) {
            device.setDepthFormat(candidates[i]);
            device.setChannelCount(sizes[i]);
            return true;
        }
    }

    return false;
}

void VulkanSwapchain::destroySwapchain(VulkanDevice& device) {
    vkDeviceWaitIdle(device.getLogicalDevice());

    const auto depthImage = static_cast<VulkanImage *>(depthTexture->data);
    depthImage->destroy(device);
    FF_Memory::ff_free_class<VulkanImage>(depthImage, sizeof(VulkanImage), TEXTURE);
    depthTexture->data = nullptr;
    FF_Memory::ff_free(depthTexture, sizeof(Texture), TEXTURE);
    depthTexture = nullptr;

    for (unsigned int i = 0; i < imageCount; i++) {
        VulkanImage& image = *static_cast<VulkanImage *>(textures[i]->data);
        vkDestroyImageView(device.getLogicalDevice(), image.getImageView(), nullptr);
    }

    if (handle) {
        vkDestroySwapchainKHR(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }

    for (unsigned int i = 0; i < imageCount; i++) {
        FF_Memory::ff_free_class<VulkanImage>(textures[i]->data, sizeof(VulkanImage), TEXTURE);
        FF_Memory::ff_free(textures[i], sizeof(Texture), TEXTURE);
    }
    textures.shutdown();

    imageCount = 0;
}
