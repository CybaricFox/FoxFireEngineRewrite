//
// Created by cmorg on 7/3/2026.
//

#include "VulkanContext.h"

#include "../../Memory/FF_Memory.h"
#include "src/modules/engine/Library/Logger.h"

int VulkanContext::findMemoryIndex(const int typeFilter, const unsigned int propertyFlags) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &memProperties);

    for (unsigned int i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) {
            return static_cast<int>(i);
        }
    }

    Logger::logWarn("Unable to find memory type!");
    return -1;
}

void VulkanContext::createCommandBuffers() {
    if (!commandBuffers) {
        commandBuffers = static_cast<VulkanCommandBuffer *>(FF_Memory::ff_allocate(sizeof(VulkanCommandBuffer) * swapchain.imageCount, ARRAY));
        for (unsigned int i = 0; i < swapchain.imageCount; i++) {
            FF_Memory::ff_clear(&commandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }
}

void VulkanContext::destroyCommandBuffers() {
    FF_Memory::ff_free(commandBuffers, sizeof(VulkanCommandBuffer) * swapchain.imageCount, ARRAY);
    commandBuffers = nullptr;
}

void VulkanContext::destroyFences() {
    FF_Memory::ff_free(inFlightFences, sizeof(VulkanFence) * swapchain.maxFramesInFlight, ARRAY);
    inFlightFences = nullptr;
    FF_Memory::ff_free(imagesInFlight, sizeof(VulkanFence) * swapchain.imageCount, ARRAY);
    imagesInFlight = nullptr;
}

void VulkanContext::createSyncObjects() {
    imageAvailableSemaphores = static_cast<VkSemaphore *>(FF_Memory::ff_allocate(sizeof(VkSemaphore) * swapchain.maxFramesInFlight, ARRAY));
    queueCompleteSemaphores = static_cast<VkSemaphore *>(FF_Memory::ff_allocate(sizeof(VkSemaphore) * swapchain.imageCount, ARRAY));
    inFlightFences = static_cast<VulkanFence *>(FF_Memory::ff_allocate(sizeof(VulkanFence) * swapchain.maxFramesInFlight, ARRAY));

    //IF AN ERROR OCCURS, THIS MIGHT BE WHY. THIS SHOULD BE CALLED SEPERATELY FROM THE OTHERS.
    imagesInFlight = static_cast<VulkanFence **>(FF_Memory::ff_allocate(sizeof(VulkanFence*) * swapchain.imageCount, ARRAY));
}

void VulkanContext::clearImagesInFlight() {
    for (unsigned int i = 0; i < swapchain.imageCount; i++) {
        imagesInFlight[i] = nullptr;
    }
}

void VulkanContext::destroyContext() {
    FF_Memory::ff_clear(&device.swapChainSupportInfo.capabilities, sizeof(device.swapChainSupportInfo.capabilities));

    device.graphicsQueueIndex = -1;
    device.presentQueueIndex = -1;
    device.transferQueueIndex = -1;

    Logger::logDebug("Destroying command pools.");
    vkDestroyCommandPool(device.logicalDevice, device.commandPool, nullptr);

    Logger::logDebug("Destroying logical device.");
    if (device.logicalDevice) {
        vkDestroyDevice(device.logicalDevice, nullptr);
        device.logicalDevice = nullptr;
    }

    Logger::logInfo("Releasing Vulkan device resources");
    device.graphicsQueue = nullptr;
    device.presentQueue = nullptr;
    device.transferQueue = nullptr;

    if (device.swapChainSupportInfo.formats) {
        FF_Memory::ff_free(device.swapChainSupportInfo.formats, sizeof(VkSurfaceFormatKHR) * device.swapChainSupportInfo.formatCount, RENDER);
        device.swapChainSupportInfo.formats = nullptr;
        device.swapChainSupportInfo.formatCount = 0;
    }

    if (device.swapChainSupportInfo.presentModes) {
        FF_Memory::ff_free(device.swapChainSupportInfo.presentModes, sizeof(VkPresentModeKHR) * device.swapChainSupportInfo.presentCount, RENDER);
        device.swapChainSupportInfo.presentModes = nullptr;
        device.swapChainSupportInfo.presentCount = 0;
    }

    device.physicalDevice = nullptr;

    Logger::logDebug("Destroying Vulkan debugger.");
    if (getDebugMessenger()) {
        const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        function(instance, *getDebugMessenger(), nullptr);
    }

    Logger::logDebug("Destroying Vulkan surface.");
    vkDestroySurfaceKHR(instance, surface, nullptr);

    Logger::logDebug("Destroying Vulkan instance.");
    vkDestroyInstance(instance, nullptr);
}

void VulkanContext::destroyRenderpass() {
    if (renderpass.handle) {
        vkDestroyRenderPass(device.logicalDevice, renderpass.handle, nullptr);
        renderpass.handle = nullptr;
    }
}
