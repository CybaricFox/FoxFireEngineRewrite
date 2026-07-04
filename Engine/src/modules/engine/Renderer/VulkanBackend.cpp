//
// Created by cmorg on 7/2/2026.
//

#include "VulkanBackend.h"

#include <cassert>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../Library/Logger.h"

VulkanContext VulkanBackend::vulkanContext{};
unsigned int VulkanBackend::cachedWidth = 0;
unsigned int VulkanBackend::cachedHeight = 0;

VkBool32 VulkanBackend::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData) {

    switch (messageSeverity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Logger::logError(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Logger::logWarn(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Logger::logInfo(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            Logger::logDebug(callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}

bool VulkanBackend::createDevice() {
    if (!selectPhysicalDevice()) {
        return false;
    }

    Logger::logInfo("Creating logical device.");
    bool presentSharesGraphicsQueue = vulkanContext.getDevice()->graphicsQueueIndex == vulkanContext.getDevice()->presentQueueIndex;
    bool transferSharesGraphicsQueue = vulkanContext.getDevice()->graphicsQueueIndex == vulkanContext.getDevice()->transferQueueIndex;
    unsigned int indexCount = 1;

    if (!presentSharesGraphicsQueue) {
        indexCount++;
    }
    if (!transferSharesGraphicsQueue) {
        indexCount++;
    }
    unsigned int indices[indexCount];
    unsigned char index = 0;
    indices[index++] = vulkanContext.getDevice()->graphicsQueueIndex;
    if (!presentSharesGraphicsQueue) {
        indices[index++] = vulkanContext.getDevice()->presentQueueIndex;
    }
    if (!transferSharesGraphicsQueue) {
        indices[index++] = vulkanContext.getDevice()->transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[indexCount];
    for (unsigned int i = 0; i < indexCount; i++) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;

        //This wont work on some gpus, ignore for now since it works on mine.
        if (indices[i] == vulkanContext.getDevice()->graphicsQueueIndex) {
            queueCreateInfos[i].queueCount = 2;
        }

        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = nullptr;
        float queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.queueCreateInfoCount = indexCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 1;
    const auto extensionNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    createInfo.ppEnabledExtensionNames = &extensionNames;

    vulkanCheck(vkCreateDevice(vulkanContext.getDevice()->physicalDevice, &createInfo, nullptr, &vulkanContext.getDevice()->logicalDevice));
    Logger::logInfo("Vulkan logical device created.");

    vkGetDeviceQueue(vulkanContext.getDevice()->logicalDevice, vulkanContext.getDevice()->graphicsQueueIndex, 0, &vulkanContext.getDevice()->graphicsQueue);
    vkGetDeviceQueue(vulkanContext.getDevice()->logicalDevice, vulkanContext.getDevice()->presentQueueIndex, 0, &vulkanContext.getDevice()->presentQueue);
    vkGetDeviceQueue(vulkanContext.getDevice()->logicalDevice, vulkanContext.getDevice()->transferQueueIndex, 0, &vulkanContext.getDevice()->transferQueue);
    Logger::logInfo("Vulkan queues obtained.");

    VkCommandPoolCreateInfo poolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.queueFamilyIndex = vulkanContext.getDevice()->graphicsQueueIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vulkanCheck(vkCreateCommandPool(vulkanContext.getDevice()->logicalDevice, &poolCreateInfo, nullptr, &vulkanContext.getDevice()->commandPool));
    Logger::logInfo("Graphics command pool created.");

    return true;
}

bool VulkanBackend::createSurface(Platform* platform) {
    return platform->createSurface();
}

void VulkanBackend::createSwapchain(const unsigned int width, const unsigned int height) {
    VkExtent2D swapchainExtent{width, height};
    vulkanContext.getSwapchain()->maxFramesInFlight = 2;

    bool found = false;
    for (unsigned int i = 0; i < vulkanContext.getDevice()->swapChainSupportInfo.formatCount; i++) {
        VkSurfaceFormatKHR surfaceFormat = vulkanContext.getDevice()->swapChainSupportInfo.formats[i];
        //This format is preferred
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            vulkanContext.getSwapchain()->imageFormat = surfaceFormat;
            found = true;
            break;
        }
    }

    //If not found, just use the first one
    if (!found) {
        vulkanContext.getSwapchain()->imageFormat = vulkanContext.getDevice()->swapChainSupportInfo.formats[0];
    }

    //FIFO is gurarnteed. Use it as the default.
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (unsigned int i = 0; i < vulkanContext.getDevice()->swapChainSupportInfo.presentCount; i++) {
        //Check for mailbox if available. This one is better.
        VkPresentModeKHR mode = vulkanContext.getDevice()->swapChainSupportInfo.presentModes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = mode;
            break;
        }
    }

    querySwapChainSupport(vulkanContext.getDevice()->physicalDevice, *vulkanContext.getSurface(), &vulkanContext.getDevice()->swapChainSupportInfo);

    if (vulkanContext.getDevice()->swapChainSupportInfo.capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent = vulkanContext.getDevice()->swapChainSupportInfo.capabilities.currentExtent;
    }

    VkExtent2D min = vulkanContext.getDevice()->swapChainSupportInfo.capabilities.minImageExtent;
    VkExtent2D max = vulkanContext.getDevice()->swapChainSupportInfo.capabilities.maxImageExtent;
    swapchainExtent.width = std::clamp(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = std::clamp(swapchainExtent.height, min.height, max.height);

    unsigned int imageCount = vulkanContext.getDevice()->swapChainSupportInfo.capabilities.minImageCount + 1;
    if (vulkanContext.getDevice()->swapChainSupportInfo.capabilities.maxImageCount > 0 && imageCount > vulkanContext.getDevice()->swapChainSupportInfo.capabilities.maxImageCount) {
        imageCount = vulkanContext.getDevice()->swapChainSupportInfo.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapChainCreateInfo.surface = *vulkanContext.getSurface();
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = vulkanContext.getSwapchain()->imageFormat.format;
    swapChainCreateInfo.imageColorSpace = vulkanContext.getSwapchain()->imageFormat.colorSpace;
    swapChainCreateInfo.imageExtent = swapchainExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const unsigned int queueFamilyIndices[] = {static_cast<unsigned int>(vulkanContext.getDevice()->graphicsQueueIndex), static_cast<unsigned int>(vulkanContext.getDevice()->presentQueueIndex)};

    if (vulkanContext.getDevice()->graphicsQueueIndex != vulkanContext.getDevice()->presentQueueIndex) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainCreateInfo.preTransform = vulkanContext.getDevice()->swapChainSupportInfo.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = nullptr;

    vulkanCheck(vkCreateSwapchainKHR(vulkanContext.getDevice()->logicalDevice, &swapChainCreateInfo, nullptr, &vulkanContext.getSwapchain()->handle));

    *vulkanContext.getCurrentFrame() = 0;
    vulkanContext.getSwapchain()->imageCount = 0;
    vulkanCheck(vkGetSwapchainImagesKHR(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->handle, &vulkanContext.getSwapchain()->imageCount, nullptr));
    if (!vulkanContext.getSwapchain()->images) {
        vulkanContext.getSwapchain()->images = static_cast<VkImage *>(FF_Memory::ff_allocate(sizeof(VkImage) * vulkanContext.getSwapchain()->imageCount, RENDER));
    }
    if (!vulkanContext.getSwapchain()->imageViews) {
        vulkanContext.getSwapchain()->imageViews = static_cast<VkImageView*>(FF_Memory::ff_allocate(sizeof(VkImageView) * vulkanContext.getSwapchain()->imageCount, RENDER));
    }
    vulkanCheck(vkGetSwapchainImagesKHR(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->handle, &vulkanContext.getSwapchain()->imageCount, vulkanContext.getSwapchain()->images));

    for (unsigned int i = 0; i < vulkanContext.getSwapchain()->imageCount; i++) {
        VkImageViewCreateInfo viewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewCreateInfo.image = vulkanContext.getSwapchain()->images[i];
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = vulkanContext.getSwapchain()->imageFormat.format;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        vulkanCheck(vkCreateImageView(vulkanContext.getDevice()->logicalDevice, &viewCreateInfo, nullptr, &vulkanContext.getSwapchain()->imageViews[i]));
    }

    if (!detectDepthFormat()) {
        vulkanContext.getDevice()->depthFormat = VK_FORMAT_UNDEFINED;
        Logger::logFatal("Failed to find a supported format!");
    }

    createImage(VK_IMAGE_TYPE_2D,
        swapchainExtent.width,
        swapchainExtent.height,
        vulkanContext.getDevice()->depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &vulkanContext.getSwapchain()->depthAttachment
        );

    Logger::logInfo("Successfully created swapchain!");
}

void VulkanBackend::recreateSwapchain(const unsigned int width, const unsigned int height) {
    destroySwapchain();
    createSwapchain(width, height);
}

bool VulkanBackend::selectPhysicalDevice() {
    unsigned int deviceCount = 0;
    vulkanCheck(vkEnumeratePhysicalDevices(*vulkanContext.getInstance(), &deviceCount, nullptr));
    if (deviceCount == 0) {
        Logger::logFatal("No devices were found that support Vulkan.");
        return false;
    }

    VkPhysicalDevice devices[deviceCount];
    vulkanCheck(vkEnumeratePhysicalDevices(*vulkanContext.getInstance(), &deviceCount, devices));

    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);

        //Requirements are request from engine. These are what the engine wants.
        PhysicalDeviceRequirements requirements{};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        requirements.samplerAnisotrophy = true;
        requirements.discreteGPU = true;
        requirements.extensionNames.clear();
        requirements.extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VulkanPhysicalDeviceFamilyInfo familyInfo{};
        const bool result = physicalDeviceMeetsRequirements(
            device,
            *vulkanContext.getSurface(),
            &deviceProperties,
            &deviceFeatures,
            &requirements,
            &familyInfo,
            &vulkanContext.getDevice()->swapChainSupportInfo
            );

        if (result) {
            Logger::logInfo("Selected device: " + String(deviceProperties.deviceName));
            switch (deviceProperties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    Logger::logInfo("GPU type is unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    Logger::logInfo("GPU type is integated GPU.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    Logger::logInfo("GPU type is discrete GPU.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    Logger::logInfo("GPU type is virtual GPU.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    Logger::logInfo("GPU type is CPU.");
                    break;
            }

            Logger::logInfo("GPU driver version: " +
                std::to_string(VK_VERSION_MAJOR(deviceProperties.driverVersion)) +
                "." +
                std::to_string(VK_VERSION_MINOR(deviceProperties.driverVersion)) +
                "." +
                std::to_string(VK_VERSION_PATCH(deviceProperties.driverVersion)));

            Logger::logInfo("Vulkan API version: " +
                std::to_string(VK_VERSION_MAJOR(deviceProperties.apiVersion)) +
                "." +
                std::to_string(VK_VERSION_MINOR(deviceProperties.apiVersion)) +
                "." +
                std::to_string(VK_VERSION_PATCH(deviceProperties.apiVersion)));

            for (unsigned int i = 0; i < deviceMemoryProperties.memoryHeapCount; i++) {
                float memorySizeGB = static_cast<float>(deviceMemoryProperties.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f;
                if (deviceMemoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    std::ostringstream oss;
                    oss << "Local GPU memory: " << std::setprecision(2) << memorySizeGB << "GB";
                    Logger::logInfo(oss.str());
                } else {
                    std::ostringstream oss;
                    oss << "Shared system memory: " << std::setprecision(2) << memorySizeGB << "GB";
                    Logger::logInfo(oss.str());
                }
            }

            vulkanContext.getDevice()->physicalDevice = device;
            vulkanContext.getDevice()->graphicsQueueIndex = static_cast<int>(familyInfo.graphicsFamily);
            vulkanContext.getDevice()->presentQueueIndex = static_cast<int>(familyInfo.presentFamily);
            vulkanContext.getDevice()->transferQueueIndex = static_cast<int>(familyInfo.transferFamily);

            vulkanContext.getDevice()->physicalDeviceProperties = deviceProperties;
            vulkanContext.getDevice()->physicalDeviceFeatures = deviceFeatures;
            vulkanContext.getDevice()->physicalDeviceMemoryProperties = deviceMemoryProperties;
            break;
        }
    }

    if (!vulkanContext.getDevice()->physicalDevice) {
        Logger::logError("Failed to find a valid physical device.");
        return false;
    }

    Logger::logInfo("Physical device selected.");
    return true;
}

bool VulkanBackend::swapchainAcquireNextImageIndex(const unsigned long timeout, VkSemaphore semaphore, VkFence fence, unsigned int *outImageIndex) {
    const VkResult result = vkAcquireNextImageKHR(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->handle, timeout, semaphore, fence, outImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(*vulkanContext.getFrameBufferWidth(), *vulkanContext.getFrameBufferHeight());
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        Logger::logFatal("Failed to acquire swapchain image!");
        return false;
    }

    return true;
}

void VulkanBackend::presentSwapchain(VkSemaphore semaphore, const unsigned int presentImageIndex) {
    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &semaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkanContext.getSwapchain()->handle;
    presentInfo.pImageIndices = &presentImageIndex;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(vulkanContext.getDevice()->presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(*vulkanContext.getFrameBufferWidth(), *vulkanContext.getFrameBufferHeight());
    } else if (result != VK_SUCCESS) {
        Logger::logFatal("Failed to present swapchain image!");
    }

    //loop the current frame
    *vulkanContext.getCurrentFrame() = (*vulkanContext.getCurrentFrame() + 1) % vulkanContext.getSwapchain()->maxFramesInFlight;
}

bool VulkanBackend::detectDepthFormat() {
    constexpr unsigned long candidateCount = 3;
    VkFormat candidates[candidateCount]{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    constexpr unsigned int flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (VkFormat format : candidates) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(vulkanContext.getDevice()->physicalDevice, format, &formatProperties);

        if ((formatProperties.linearTilingFeatures & flags) == flags || (formatProperties.optimalTilingFeatures & flags) == flags) {
            vulkanContext.getDevice()->depthFormat = format;
            return true;
        }
    }

    return false;
}

void VulkanBackend::beginRenderpass(VulkanCommandBuffer *commandBuffer, const VkFramebuffer frameBuffer) {
    VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = vulkanContext.getRenderpass()->handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = static_cast<int32_t>(vulkanContext.getRenderpass()->x);
    beginInfo.renderArea.offset.y = static_cast<int32_t>(vulkanContext.getRenderpass()->y);
    beginInfo.renderArea.extent.width = static_cast<int32_t>(vulkanContext.getRenderpass()->w);
    beginInfo.renderArea.extent.height = static_cast<int32_t>(vulkanContext.getRenderpass()->h);

    VkClearValue clearValues[2];
    FF_Memory::ff_clear(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = vulkanContext.getRenderpass()->r;
    clearValues[0].color.float32[1] = vulkanContext.getRenderpass()->g;
    clearValues[0].color.float32[2] = vulkanContext.getRenderpass()->b;
    clearValues[0].color.float32[3] = vulkanContext.getRenderpass()->a;
    clearValues[1].depthStencil.depth = vulkanContext.getRenderpass()->depth;
    clearValues[1].depthStencil.stencil = vulkanContext.getRenderpass()->stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = IN_RENDER_PASS;
}

void VulkanBackend::endRenderpass(VulkanCommandBuffer *commandBuffer) {
    vkCmdEndRenderPass(commandBuffer->handle);
    commandBuffer->state = RECORDING;
}

void VulkanBackend::allocateCommandBuffer(const bool bIsPrimary, VulkanCommandBuffer *commandBuffer) {
    FF_Memory::ff_clear(commandBuffer, sizeof(commandBuffer));

    VkCommandBufferAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = vulkanContext.getDevice()->commandPool;
    allocateInfo.level = bIsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = nullptr;

    commandBuffer->state = NOT_ALLOCATED;
    vulkanCheck(vkAllocateCommandBuffers(vulkanContext.getDevice()->logicalDevice, &allocateInfo, &commandBuffer->handle));
    commandBuffer->state = READY;
}

void VulkanBackend::freeCommandBuffer(VulkanCommandBuffer *commandBuffer) {
    vkFreeCommandBuffers(vulkanContext.getDevice()->logicalDevice, vulkanContext.getDevice()->commandPool, 1, &commandBuffer->handle);
    commandBuffer->handle = nullptr;
    commandBuffer->state = NOT_ALLOCATED;
}

void VulkanBackend::beginCommandBuffer(VulkanCommandBuffer* commandBuffer, const bool bIsSingleUse, const bool bIsRenderpassContinue, const bool bIsConcurrent) {
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

    vulkanCheck(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo));
    commandBuffer->state = RECORDING;
}

void VulkanBackend::endCommandBuffer(VulkanCommandBuffer* commandBuffer) {
    vulkanCheck(vkEndCommandBuffer(commandBuffer->handle));
    commandBuffer->state = RECORDING_ENDED;
}

void VulkanBackend::updateSubmittedCommandBuffer(VulkanCommandBuffer *commandBuffer) {
    commandBuffer->state = SUBMITTED;
}

void VulkanBackend::resetCommandBuffer(VulkanCommandBuffer *commandBuffer) {
    commandBuffer->state = READY;
}

void VulkanBackend::allocateAndBeginSingleUseCommandBuffer(VulkanCommandBuffer *commandBuffer) {
    allocateCommandBuffer(true, commandBuffer);
    beginCommandBuffer(commandBuffer, true, false, false);
}

void VulkanBackend::endSingleUseCommandBuffer(VulkanCommandBuffer *commandBuffer, VkQueue queue) {
    endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    vulkanCheck(vkQueueSubmit(queue, 1, &submitInfo, nullptr));

    //Wait for queue to finish since there is no fence here
    vulkanCheck(vkQueueWaitIdle(queue));

    freeCommandBuffer(commandBuffer);
}

void VulkanBackend::allocateCommandBuffers() {
    vulkanContext.createCommandBuffers();

    for (unsigned int i = 0; i < vulkanContext.getSwapchain()->imageCount; i++) {
        if (vulkanContext.getCommandBuffer(i)->handle) {
            freeCommandBuffer(vulkanContext.getCommandBuffer(i));
        }
        FF_Memory::ff_clear(vulkanContext.getCommandBuffer(i), sizeof(VulkanCommandBuffer));
        allocateCommandBuffer(true, vulkanContext.getCommandBuffer(i));
    }

    Logger::logInfo("Vulkan command buffers created and allocated.");
}

void VulkanBackend::createFramebuffer(const unsigned int width, const unsigned int height, unsigned int attachmentCount, VkImageView* view, VulkanFramebuffer *framebuffer) {
    framebuffer->attachments = static_cast<VkImageView*>(FF_Memory::ff_allocate(sizeof(VkImageView) * attachmentCount, RENDER));
    for (unsigned int i = 0; i < attachmentCount; i++) {
        framebuffer->attachments[i] = view[i];
    }

    framebuffer->renderpass = vulkanContext.getRenderpass();
    framebuffer->attachmentCount = attachmentCount;

    VkFramebufferCreateInfo frameBufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    frameBufferCreateInfo.renderPass = vulkanContext.getRenderpass()->handle;
    frameBufferCreateInfo.attachmentCount = attachmentCount;
    frameBufferCreateInfo.pAttachments = framebuffer->attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    vulkanCheck(vkCreateFramebuffer(vulkanContext.getDevice()->logicalDevice, &frameBufferCreateInfo, nullptr, &framebuffer->handle));
}

void VulkanBackend::regenerateFramebuffers() {
    for (unsigned int i = 0; i < vulkanContext.getSwapchain()->imageCount; i++) {
        constexpr unsigned int attachmentCount = 2;
        VkImageView attachments[]{vulkanContext.getSwapchain()->imageViews[i], vulkanContext.getSwapchain()->depthAttachment.view};

        createFramebuffer(*vulkanContext.getFrameBufferWidth(), *vulkanContext.getFrameBufferHeight(), attachmentCount, attachments, &vulkanContext.getSwapchain()->framebuffers[i]);
    }
}

void VulkanBackend::destroyFramebuffer(VulkanFramebuffer *framebuffer) {
    //Destroy frame buffers
    vkDestroyFramebuffer(vulkanContext.getDevice()->logicalDevice, framebuffer->handle, nullptr);
    if (framebuffer->attachments) {
        FF_Memory::ff_free(framebuffer->attachments, sizeof(VkImageView) * framebuffer->attachmentCount, RENDER);
        framebuffer->attachments = nullptr;
    }
    framebuffer->handle = nullptr;
    framebuffer->attachmentCount = 0;
    framebuffer->renderpass = nullptr;
}

void VulkanBackend::createFence(bool bCreateSignaled, VulkanFence * fence) {
    fence->bIsSignaled = bCreateSignaled;
    VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (fence->bIsSignaled) {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    vulkanCheck(vkCreateFence(vulkanContext.getDevice()->logicalDevice, &fenceCreateInfo, nullptr, &fence->handle));
}

void VulkanBackend::destroyFence(VulkanFence *fence) {
        if (fence->handle) {
            vkDestroyFence(vulkanContext.getDevice()->logicalDevice, fence->handle, nullptr);
            fence->handle = nullptr;
        }
    fence->bIsSignaled = false;
}

bool VulkanBackend::waitForFence(VulkanFence *fence, const unsigned long timeout) {
    if (!fence->bIsSignaled) {
        switch (vkWaitForFences(vulkanContext.getDevice()->logicalDevice, 1, &fence->handle, true, timeout)) {
            case VK_SUCCESS:
                fence->bIsSignaled = true;
                return true;
            case VK_TIMEOUT:
                Logger::logWarn("Fence timed out.");
                break;
            case VK_ERROR_DEVICE_LOST:
                Logger::logError("Fence lost device.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                Logger::logError("Fence ran out of host memory.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                Logger::logError("Fence ran out of device memory.");
                break;
            default:
                Logger::logError("Fence encountered an unknown error.");
                break;
        }
    } else {
        return true;
    }

    return false;
}

void VulkanBackend::resetFence(VulkanFence *fence) {
    if (fence->bIsSignaled) {
        vulkanCheck(vkResetFences(vulkanContext.getDevice()->logicalDevice, 1, &fence->handle));
        fence->bIsSignaled = false;
    }
}


bool VulkanBackend::physicalDeviceMeetsRequirements(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                    const VkPhysicalDeviceProperties *deviceProperties, const VkPhysicalDeviceFeatures *deviceFeatures,
                                                    const PhysicalDeviceRequirements *requirements, VulkanPhysicalDeviceFamilyInfo *physicalDeviceFamilyInfo,
                                                    VulkanSwapChainSupportInfo *swapChainSupport) {
    physicalDeviceFamilyInfo->graphicsFamily = -1;
    physicalDeviceFamilyInfo->presentFamily = -1;
    physicalDeviceFamilyInfo->computeFamily = -1;
    physicalDeviceFamilyInfo->transferFamily = -1;

    if (requirements->discreteGPU) {
        if (deviceProperties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            Logger::logInfo(String(deviceProperties->deviceName) + " is not a discrete GPU. Skipping to next device.");
            return false;
        }
    }

    unsigned int familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    VkQueueFamilyProperties queueFamilyProperties[familyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, queueFamilyProperties);

    std::ostringstream oss1;
    oss1 << std::setw(8) << "Graphics" << std::setw(3) << " | " <<
        std::setw(7) << "Present" << std::setw(3) << " | " <<
            std::setw(7) << "Compute" << std::setw(3) << " | " <<
                std::setw(8) << "Transfer" << std::setw(3) << " | " <<
                    "Name";
    Logger::logInfo(oss1.str());

    //Whether this device meets requirements
    //If the requirement is set to false, set the validity to true since we can ignore it
    //If the requirement is set to true, set the validity to false so the below loop can check if it is valid
    bool validGraphics = !requirements->graphics;
    bool validPresent = !requirements->present;
    bool validTransfer = !requirements->transfer;
    bool validCompute = !requirements->compute;

    unsigned char minTransferScore = 255;
    for (unsigned int i = 0; i < familyCount; i++) {
        unsigned char currentScore = 0;

        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            physicalDeviceFamilyInfo->graphicsFamily = i;
            validGraphics = true;
            ++currentScore;
        }
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            physicalDeviceFamilyInfo->computeFamily = i;
            validCompute = true;
            ++currentScore;
        }
        //Since we want the dedicated transfer family on the gpu, the lower the index, the more likely it's what we're looking for.
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if (currentScore <= minTransferScore) {
                minTransferScore = currentScore;
                physicalDeviceFamilyInfo->transferFamily = i;
                validTransfer = true;
            }
        }
        VkBool32 supportsPresent = VK_FALSE;
        vulkanCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent));
        if (supportsPresent) {
            physicalDeviceFamilyInfo->presentFamily = i;
            validPresent = true;
        }
    }

    std::ostringstream oss2;
    oss2 << std::setw(8) << std::to_string(physicalDeviceFamilyInfo->graphicsFamily != -1) << std::setw(3) << " | " <<
        std::setw(7) << std::to_string(physicalDeviceFamilyInfo->presentFamily != -1) << std::setw(3) << " | " <<
            std::setw(7) << std::to_string(physicalDeviceFamilyInfo->computeFamily != -1) << std::setw(3) << " | " <<
                std::setw(8) << std::to_string(physicalDeviceFamilyInfo->transferFamily != -1) << std::setw(3) << " | " <<
                    deviceProperties->deviceName;
    Logger::logInfo(oss2.str());
    oss2.clear();

    if (validGraphics && validPresent && validTransfer && validCompute) {
        Logger::logInfo(String(deviceProperties->deviceName) + " meets requirements.");
        Logger::logDebug("Graphics family index: " + std::to_string(physicalDeviceFamilyInfo->graphicsFamily));
        Logger::logDebug("Present family index: " + std::to_string(physicalDeviceFamilyInfo->presentFamily));
        Logger::logDebug("Transfer family index: " + std::to_string(physicalDeviceFamilyInfo->transferFamily));
        Logger::logDebug("Compute family index: " + std::to_string(physicalDeviceFamilyInfo->computeFamily));

        querySwapChainSupport(physicalDevice, surface, swapChainSupport);

        if (swapChainSupport->formatCount < 1 || swapChainSupport->presentCount < 1) {
            if (swapChainSupport->formats) {
                FF_Memory::ff_free(swapChainSupport->formats, sizeof(VkSurfaceFormatKHR) * swapChainSupport->formatCount, RENDER);
            }
            if (swapChainSupport->presentModes) {
                FF_Memory::ff_free(swapChainSupport->presentModes, sizeof(VkPresentModeKHR) * swapChainSupport->presentCount, RENDER);
            }
            Logger::logInfo("Nevermind, Swap chain is not supported by this device. Skipping to next device.");
            return false;
        }

        if (!requirements->extensionNames.empty()) {
            unsigned int extensionCount = 0;
            VkExtensionProperties *availableExtensions = nullptr;
            vulkanCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
            if (extensionCount != 0) {
                availableExtensions = static_cast<VkExtensionProperties *>(FF_Memory::ff_allocate(sizeof(VkExtensionProperties) * extensionCount, RENDER));
                vulkanCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions));
                for (const char* extension : requirements->extensionNames) {
                    bool found = false;
                    for (unsigned int i = 0; i < extensionCount; i++) {
                        if (strcmp(extension, availableExtensions[i].extensionName) == 0) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        Logger::logInfo(String(extension) + " not found. Skipping device.");
                        FF_Memory::ff_free(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, RENDER);
                        return false;
                    }
                }
            }
            FF_Memory::ff_free(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, RENDER);
        }

        if (requirements->samplerAnisotrophy && !deviceFeatures->samplerAnisotropy) {
            Logger::logInfo("Device does not support sampler anisotrophy, skipping.");
            return false;
        }

        return true;
    }

    return false;
}

void VulkanBackend::createImage(VkImageType imageType, const unsigned int width, const unsigned int height, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, bool createView,
    VkImageAspectFlags aspect, VulkanImage *outImage) {

    outImage->width = width;
    outImage->height = height;

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

    vulkanCheck(vkCreateImage(vulkanContext.getDevice()->logicalDevice, &imageCreateInfo, nullptr, &outImage->handle));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vulkanContext.getDevice()->logicalDevice, outImage->handle, &memRequirements);
    const int memoryType = vulkanContext.findMemoryIndex(static_cast<int>(memRequirements.memoryTypeBits), memoryPropertyFlags);
    if (memoryType == -1) {
        Logger::logError("Memory type could not be found. The image in invalid.");
        return;
    }

    VkMemoryAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = memoryType;
    vulkanCheck(vkAllocateMemory(vulkanContext.getDevice()->logicalDevice, &allocateInfo, nullptr, &outImage->deviceMemory));
    vulkanCheck(vkBindImageMemory(vulkanContext.getDevice()->logicalDevice, outImage->handle, outImage->deviceMemory, 0));

    if (createView) {
        outImage->view = nullptr;
        createImageView(format, outImage, aspect);
    }
}

void VulkanBackend::querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VulkanSwapChainSupportInfo *swapChainSupportInfo) {
    vulkanCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainSupportInfo->capabilities));

    vulkanCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &swapChainSupportInfo->formatCount, nullptr));
    if (swapChainSupportInfo->formatCount != 0) {
        if (!swapChainSupportInfo->formats) {
            swapChainSupportInfo->formats = static_cast<VkSurfaceFormatKHR*>(FF_Memory::ff_allocate(sizeof(VkSurfaceFormatKHR) * swapChainSupportInfo->formatCount, RENDER));
        }
        vulkanCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &swapChainSupportInfo->formatCount, swapChainSupportInfo->formats));
    }

    vulkanCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &swapChainSupportInfo->presentCount, nullptr));
    if (swapChainSupportInfo->presentCount != 0) {
        if (!swapChainSupportInfo->presentModes) {
            swapChainSupportInfo->presentModes = static_cast<VkPresentModeKHR *>(FF_Memory::ff_allocate(sizeof(VkPresentModeKHR) * swapChainSupportInfo->presentCount, RENDER));
        }
        vulkanCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &swapChainSupportInfo->presentCount, swapChainSupportInfo->presentModes));
    }
}

void VulkanBackend::createImageView(VkFormat format, VulkanImage *image, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCreateInfo.image = image->handle;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    vulkanCheck(vkCreateImageView(vulkanContext.getDevice()->logicalDevice, &viewCreateInfo, nullptr, &image->view));
}

void VulkanBackend::destroySwapchain() {
    if (vulkanContext.getSwapchain()->depthAttachment.view) {
        vkDestroyImageView(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->depthAttachment.view, nullptr);
        vulkanContext.getSwapchain()->depthAttachment.view = nullptr;
    }
    if (vulkanContext.getSwapchain()->depthAttachment.handle) {
        vkDestroyImage(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->depthAttachment.handle, nullptr);
        vulkanContext.getSwapchain()->depthAttachment.handle = nullptr;
    }
    if (vulkanContext.getSwapchain()->depthAttachment.deviceMemory) {
        vkFreeMemory(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->depthAttachment.deviceMemory, nullptr);
        vulkanContext.getSwapchain()->depthAttachment.deviceMemory = nullptr;
    }

    for (unsigned int i = 0; i < vulkanContext.getSwapchain()->imageCount; i++) {
        vkDestroyImageView(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->imageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(vulkanContext.getDevice()->logicalDevice, vulkanContext.getSwapchain()->handle, nullptr);
}

void VulkanBackend::createRenderpass(float x, float y, float w, float h, float r, float g, float b, float a, float depth, unsigned int stencil) {
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    unsigned int attachmentCount = 2;
    VkAttachmentDescription attachments[attachmentCount];

    //Color attachment
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = vulkanContext.getSwapchain()->imageFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.flags = 0;

    attachments[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    //Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = vulkanContext.getDevice()->depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments[1] = depthAttachment;

    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    //Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;

    //Multisampling
    subpass.pResolveAttachments = nullptr;

    //Attachements not used in this subpass but are needed for the next subpass
    subpass.preserveAttachmentCount = 0;
    subpass.pResolveAttachments = nullptr;

    //Dependencies for render pass
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    //Create render pass
    VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = attachmentCount;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;

    vulkanCheck(vkCreateRenderPass(vulkanContext.getDevice()->logicalDevice, &renderPassCreateInfo, nullptr, &vulkanContext.getRenderpass()->handle));
}

VulkanBackend::~VulkanBackend() {
    vkDeviceWaitIdle(vulkanContext.getDevice()->logicalDevice);

    Logger::logDebug("Destroying sync objects");
    //Destroy sync objects
    for (unsigned char i = 0; i < vulkanContext.getSwapchain()->maxFramesInFlight; i++) {
        if (vulkanContext.getImageAvailableSemaphores()[i]) {
            vkDestroySemaphore(vulkanContext.getDevice()->logicalDevice, vulkanContext.getImageAvailableSemaphores()[i], nullptr);
            vulkanContext.getImageAvailableSemaphores()[i] = nullptr;
        }
        if (vulkanContext.getQueueCompleteSemaphores()[i]) {
            vkDestroySemaphore(vulkanContext.getDevice()->logicalDevice, vulkanContext.getQueueCompleteSemaphores()[i], nullptr);
            vulkanContext.getQueueCompleteSemaphores()[i] = nullptr;
        }
        destroyFence(&vulkanContext.getInFlightFences()[i]);
    }
    FF_Memory::ff_free(vulkanContext.getImageAvailableSemaphores(), sizeof(VkSemaphore) * vulkanContext.getSwapchain()->maxFramesInFlight, ARRAY);
    *vulkanContext.getImageAvailableSemaphores() = nullptr;
    FF_Memory::ff_free(vulkanContext.getQueueCompleteSemaphores(), sizeof(VkSemaphore) * vulkanContext.getSwapchain()->maxFramesInFlight, ARRAY);
    *vulkanContext.getQueueCompleteSemaphores() = nullptr;
    vulkanContext.destroyFences();

    Logger::logDebug("Destroying command buffers.");
    //Destroy command buffers
    for (unsigned int i = 0; i < vulkanContext.getSwapchain()->imageCount; i++) {
        if (vulkanContext.getCommandBuffer(i)->handle) {
            freeCommandBuffer(vulkanContext.getCommandBuffer(i));
            vulkanContext.getCommandBuffer(i)->handle = nullptr;
        }
    }
    vulkanContext.destroyCommandBuffers();

    Logger::logDebug("Destroying frame buffers.");
    for (unsigned int i = 0; i < vulkanContext.getSwapchain()->imageCount; i++) {
        destroyFramebuffer(&vulkanContext.getSwapchain()->framebuffers[i]);
    }

    Logger::logDebug("Destroying Renderpass.");
    vulkanContext.destroyRenderpass();
    Logger::logDebug("Destroying Swapchain.");
    destroySwapchain();
    vulkanContext.destroyContext();
}

void VulkanBackend::vulkanCheck(VkResult result) {
    assert(result == VK_SUCCESS);
}

bool VulkanBackend::initialize(const String appName, Platform* platform, const unsigned int width, const unsigned int height) {
    cachedWidth = width;
    cachedHeight = height;

    vulkanContext.setWidth(cachedWidth != 0 ? cachedWidth : 800);
    vulkanContext.setHeight(cachedHeight != 0 ? cachedHeight : 600);

    cachedWidth = 0;
    cachedHeight = 0;

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
    appInfo.pEngineName = "FoxFire Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

    //Get required extensions
    std::vector<const char*> requiredExtensions{};
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    platform->getRequiredExtensions(requiredExtensions);
#if ENABLE_DEBUG_LOGGING == true
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    Logger::logDebug("Required Extensions: ");
    for (const String& extension : requiredExtensions) {
        Logger::logDebug(extension);
    }
#endif

    std::vector<const char*> validationLayers{};
    unsigned int layerCount = 0;

#if ENABLE_DEBUG_LOGGING == true
    Logger::logDebug("Debug mode enable. Starting validation layers.");

    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    layerCount = validationLayers.size();
    unsigned int availableLayerCount = 0;
    vulkanCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
    VkLayerProperties availableLayers[availableLayerCount];
    vulkanCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

    for (const char* requiredLayer : validationLayers) {
        Logger::logInfo("Searching for " + String(requiredLayer));
        bool found = false;
        for (const VkLayerProperties properties : availableLayers) {
            if (strcmp(requiredLayer, properties.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            Logger::logFatal("Validation layer " + String(requiredLayer) + " could not be found.");
            return false;
        }
    }

    Logger::logDebug("All required validation layers were found!");

#endif

    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = validationLayers.data();

    vulkanCheck(vkCreateInstance(&createInfo, nullptr, vulkanContext.getInstance()));
    Logger::logInfo("Vulkan Instance Created Successfully.");

#if ENABLE_DEBUG_LOGGING == true
    Logger::logDebug("Creating Vulkan debugger.");
    constexpr unsigned int logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

    const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(*vulkanContext.getInstance(), "vkCreateDebugUtilsMessengerEXT"));
    assert(function);
    vulkanCheck(function(*vulkanContext.getInstance(), &debugCreateInfo, nullptr, vulkanContext.getDebugMessenger()));
    Logger::logDebug("Vulkan debugger created successfully.");
#endif

    //Create surface
    Logger::logInfo("Creating Vulkan surface.");
    if (!createSurface(platform)) {
        Logger::logFatal("Failed to create surface for Vulkan.");
        return false;
    }
    Logger::logInfo("Created Vulkan surface successfully.");

    //Create device
    if (!createDevice()) {
        Logger::logFatal("Failed to create Vulkan device.");
        return false;
    }

    createSwapchain(*vulkanContext.getFrameBufferWidth(), *vulkanContext.getFrameBufferHeight());

    createRenderpass(0, 0, static_cast<float>(*vulkanContext.getFrameBufferWidth()), static_cast<float>(*vulkanContext.getFrameBufferHeight()), 0, 0, 0.2f, 1, 1, 0);

    vulkanContext.getSwapchain()->framebuffers = static_cast<VulkanFramebuffer *>(FF_Memory::ff_allocate(sizeof(VulkanFramebuffer) * vulkanContext.getSwapchain()->imageCount, ARRAY));
    regenerateFramebuffers();

    Logger::logInfo("Creating and allocating command buffers");
    allocateCommandBuffers();

    //Sync objects
    Logger::logInfo("Creating fences");
    vulkanContext.createSyncObjects();

    for (unsigned char i = 0; i < vulkanContext.getSwapchain()->maxFramesInFlight; i++) {
        VkSemaphoreCreateInfo semCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(vulkanContext.getDevice()->logicalDevice, &semCreateInfo, nullptr, &vulkanContext.getImageAvailableSemaphores()[i]);
        vkCreateSemaphore(vulkanContext.getDevice()->logicalDevice, &semCreateInfo, nullptr, &vulkanContext.getQueueCompleteSemaphores()[i]);
        createFence(true, &vulkanContext.getInFlightFences()[i]);
    }

    //Set initial state to 0. This is allocated in createSyncObject().
    for (unsigned int i = 0; i < vulkanContext.getSwapchain()->imageCount; i++) {
        FF_Memory::ff_clear(&vulkanContext.getImagesInFlight()[i], sizeof(VulkanFence));
    }

    Logger::logInfo("Vulkan renderer initialized");
    return RendererBackend::initialize(appName, platform, width, height);
}

void VulkanBackend::setVersion(const GameInstance *gameInstance) {
    majorVersion = gameInstance->config.gameVersionMajor;
    minorVersion = gameInstance->config.gameVersionMinor;
    patchVersion = gameInstance->config.gameVersionPatch;
}
