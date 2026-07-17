//
// Created by cmorg on 7/3/2026.
//

#include "VulkanContext.h"

#include "VulkanBackend.h"
#include "../../Memory/FF_Memory.h"
#include "src/modules/engine/Library/Logger.h"

void VulkanContext::createCommandBuffers() {
    if (!commandBuffers) {
        commandBuffers = static_cast<VulkanCommandBuffer *>(FF_Memory::ff_allocate(sizeof(VulkanCommandBuffer) * swapchain.getImageCount(), ARRAY));
        for (unsigned int i = 0; i < swapchain.getImageCount(); i++) {
            FF_Memory::ff_clear(&commandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }
}

void VulkanContext::destroyCommandBuffers() {
    FF_Memory::ff_free(commandBuffers, sizeof(VulkanCommandBuffer) * swapchain.getImageCount(), ARRAY);
    commandBuffers = nullptr;
}

void VulkanContext::destroyFences() {
    //FF_Memory::ff_free(inFlightFences, sizeof(VulkanFence) * swapchain.maxFramesInFlight, ARRAY);
    //inFlightFences = nullptr;
    //FF_Memory::ff_free(imagesInFlight, sizeof(VulkanFence*) * swapchain.imageCount, ARRAY);
    //imagesInFlight = nullptr;
}

void VulkanContext::createSyncObjects() {
    imageAvailableSemaphores = static_cast<VkSemaphore *>(FF_Memory::ff_allocate(sizeof(VkSemaphore) * swapchain.getMaxFramesInFlight(), ARRAY));
    queueCompleteSemaphores = static_cast<VkSemaphore *>(FF_Memory::ff_allocate(sizeof(VkSemaphore) * swapchain.getImageCount(), ARRAY));
    inFlightFences = static_cast<VulkanFence *>(FF_Memory::ff_allocate(sizeof(VulkanFence) * swapchain.getMaxFramesInFlight(), ARRAY));

    //IF AN ERROR OCCURS, THIS MIGHT BE WHY. THIS SHOULD BE CALLED SEPERATELY FROM THE OTHERS.
    imagesInFlight = static_cast<VulkanFence **>(FF_Memory::ff_allocate(sizeof(VulkanFence*) * swapchain.getImageCount(), ARRAY));
}

void VulkanContext::destroySyncObjects() {
    if (!device.getLogicalDevice()) return;

    //Destroy sync objects
    if (imageAvailableSemaphores) {
        for (unsigned char i = 0; i < swapchain.getMaxFramesInFlight(); i++) {
            if (imageAvailableSemaphores[i]) {
                vkDestroySemaphore(device.getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
                imageAvailableSemaphores[i] = nullptr;
            }
        }
        FF_Memory::ff_free(imageAvailableSemaphores, sizeof(VkSemaphore) * swapchain.getMaxFramesInFlight(), ARRAY);
        imageAvailableSemaphores = nullptr;
    }

    if (queueCompleteSemaphores) {
        for (unsigned int i = 0; i < swapchain.getImageCount(); i++) {
            if (queueCompleteSemaphores[i]) {
                vkDestroySemaphore(device.getLogicalDevice(), queueCompleteSemaphores[i], nullptr);
                queueCompleteSemaphores[i] = nullptr;
            }
        }
        FF_Memory::ff_free(queueCompleteSemaphores, sizeof(VkSemaphore) * swapchain.getImageCount(), ARRAY);
        queueCompleteSemaphores = nullptr;
    }

    if (inFlightFences) {
        for (unsigned char i = 0; i < swapchain.getMaxFramesInFlight(); i++) {
            if (inFlightFences[i].handle) {
                vkDestroyFence(device.getLogicalDevice(), inFlightFences[i].handle, nullptr);
                inFlightFences[i].handle = nullptr;
            }
            inFlightFences[i].bIsSignaled = false;
        }
        FF_Memory::ff_free(inFlightFences, sizeof(VulkanFence) * swapchain.getMaxFramesInFlight(), ARRAY);
        inFlightFences = nullptr;
        inFlightFenceCount = 0;
    }

    if (imagesInFlight) {
        FF_Memory::ff_free(imagesInFlight, sizeof(VulkanFence*) * swapchain.getImageCount(), ARRAY);
        imagesInFlight = nullptr;
    }
}

void VulkanContext::clearImagesInFlight() {
    for (unsigned int i = 0; i < swapchain.getImageCount(); i++) {
        imagesInFlight[i] = nullptr;
    }
}

void VulkanContext::destroyContext() {
    FF_Memory::ff_clear(&device.getSwapChainSupportInfo().capabilities, sizeof(device.getSwapChainSupportInfo().capabilities));

    device.getGraphicsQueueIndex() = -1;
    device.getPresentQueueIndex() = -1;
    device.getTransferQueueIndex() = -1;

    if (device.getCommandPool()) {
        Logger::logDebug("Destroying command pools.");
        vkDestroyCommandPool(device.getLogicalDevice(), device.getCommandPool(), nullptr);
        device.getCommandPool() = nullptr;
    }

    Logger::logDebug("Destroying logical device.");
    if (device.getLogicalDevice()) {
        vkDestroyDevice(device.getLogicalDevice(), nullptr);
        device.getLogicalDevice() = nullptr;
    }

    Logger::logInfo("Releasing Vulkan device resources");
    device.getGraphicsQueue() = nullptr;
    device.getPresentQueue() = nullptr;
    device.getTransferQueue() = nullptr;

    if (device.getSwapChainSupportInfo().formats) {
        FF_Memory::ff_free(device.getSwapChainSupportInfo().formats, sizeof(VkSurfaceFormatKHR) * device.getSwapChainSupportInfo().formatCount, RENDER);
        device.getSwapChainSupportInfo().formats = nullptr;
        device.getSwapChainSupportInfo().formatCount = 0;
    }

    if (device.getSwapChainSupportInfo().presentModes) {
        FF_Memory::ff_free(device.getSwapChainSupportInfo().presentModes, sizeof(VkPresentModeKHR) * device.getSwapChainSupportInfo().presentCount, RENDER);
        device.getSwapChainSupportInfo().presentModes = nullptr;
        device.getSwapChainSupportInfo().presentCount = 0;
    }

    device.getPhysicalDevice() = nullptr;

    Logger::logDebug("Destroying Vulkan debugger.");
    if (debugMessenger != nullptr) {
        if (const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))) {
            function(instance, debugMessenger, nullptr);
        }

        debugMessenger = nullptr;
    }

    if (surface) {
        Logger::logDebug("Destroying Vulkan surface.");
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = nullptr;
    }

    if (instance) {
        Logger::logDebug("Destroying Vulkan instance.");
        vkDestroyInstance(instance, nullptr);
        instance = nullptr;
    }
}

void VulkanContext::destroyRenderpass() {
    if (renderpass.handle) {
        vkDestroyRenderPass(device.getLogicalDevice(), renderpass.handle, nullptr);
        renderpass.handle = nullptr;
    }
}

void VulkanContext::allocateAndBeginSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    allocateCommandBuffer(true, commandBuffer);
    beginCommandBuffer(commandBuffer, true, false, false);
}

void VulkanContext::allocateCommandBuffer(const bool bIsPrimary, VulkanCommandBuffer& commandBuffer) {
    FF_Memory::ff_clear(&commandBuffer, sizeof(VulkanCommandBuffer));

    VkCommandBufferAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = device.getCommandPool();
    allocateInfo.level = bIsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = nullptr;

    commandBuffer.state = NOT_ALLOCATED;
    VulkanUtils::vulkanCheck(vkAllocateCommandBuffers(device.getLogicalDevice(), &allocateInfo, &commandBuffer.handle));
    commandBuffer.state = READY;
}

void VulkanContext::beginCommandBuffer(VulkanCommandBuffer& commandBuffer, const bool bIsSingleUse, const bool bIsRenderpassContinue, const bool bIsConcurrent) {
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

    VulkanUtils::vulkanCheck(vkBeginCommandBuffer(commandBuffer.handle, &beginInfo));
    commandBuffer.state = RECORDING;
}

void VulkanContext::endSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer, VkQueue queue) {
    endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.handle;

    VulkanUtils::vulkanCheck(vkQueueSubmit(queue, 1, &submitInfo, nullptr));

    //Wait for queue to finish since there is no fence here
    VulkanUtils::vulkanCheck(vkQueueWaitIdle(queue));

    freeCommandBuffer(commandBuffer);
}

void VulkanContext::freeCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    vkFreeCommandBuffers(device.getLogicalDevice(), device.getCommandPool(), 1, &commandBuffer.handle);
    commandBuffer.handle = nullptr;
    commandBuffer.state = NOT_ALLOCATED;
}

void VulkanContext::endCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    VulkanUtils::vulkanCheck(vkEndCommandBuffer(commandBuffer.handle));
    commandBuffer.state = RECORDING_ENDED;
}
