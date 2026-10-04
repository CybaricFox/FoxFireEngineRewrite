//
// Created by cmorg on 7/3/2026.
//

#include "VulkanContext.h"

#include "VulkanBackend.h"
#include "../../Memory/FF_Memory.h"
#include "src/modules/engine/Library/Logger.h"

void VulkanContext::createCommandBuffers() {
    if (!commandBuffers) {
        commandBuffers = static_cast<VulkanCommandBuffer *>(FF_Memory::ff_allocate(sizeof(VulkanCommandBuffer) * swapchain.getImageCount(), ARRAY, alignof(VulkanCommandBuffer)));
        for (unsigned int i = 0; i < swapchain.getImageCount(); i++) {
            FF_Memory::ff_clear(&commandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }
}

void VulkanContext::destroyCommandBuffers() {
    FF_Memory::ff_free(commandBuffers, sizeof(VulkanCommandBuffer) * swapchain.getImageCount(), ARRAY);
    commandBuffers = nullptr;
}

void VulkanContext::createSyncObjects() {
    imageAvailableSemaphores = static_cast<VkSemaphore *>(FF_Memory::ff_allocate(sizeof(VkSemaphore) * swapchain.getMaxFramesInFlight(), ARRAY));
    queueCompleteSemaphores = static_cast<VkSemaphore *>(FF_Memory::ff_allocate(sizeof(VkSemaphore) * swapchain.getImageCount(), ARRAY));

    imagesInFlight.initialize(swapchain.getImageCount());
}

void VulkanContext::destroySyncObjects() {
    if (!device.getLogicalDevice()) return;

    //Destroy sync objects
    if (imageAvailableSemaphores) {
        for (unsigned char i = 0; i < swapchain.getMaxFramesInFlight(); i++) {
            if (imageAvailableSemaphores[i]) {
                vkDestroySemaphore(device.getLogicalDevice(), imageAvailableSemaphores[i], allocator);
                imageAvailableSemaphores[i] = nullptr;
            }

            vkDestroyFence(device.getLogicalDevice(), inFlightFences[i], allocator);
        }
        FF_Memory::ff_free(imageAvailableSemaphores, sizeof(VkSemaphore) * swapchain.getMaxFramesInFlight(), ARRAY);
        imageAvailableSemaphores = nullptr;
    }

    if (queueCompleteSemaphores) {
        for (unsigned int i = 0; i < swapchain.getImageCount(); i++) {
            if (queueCompleteSemaphores[i]) {
                vkDestroySemaphore(device.getLogicalDevice(), queueCompleteSemaphores[i], allocator);
                queueCompleteSemaphores[i] = nullptr;
            }
        }
        FF_Memory::ff_free(queueCompleteSemaphores, sizeof(VkSemaphore) * swapchain.getImageCount(), ARRAY);
        queueCompleteSemaphores = nullptr;
    }

    imagesInFlight.shutdown();
}

void VulkanContext::clearImagesInFlight() {
    for (unsigned int i = 0; i < swapchain.getImageCount(); i++) {
        imagesInFlight[i] = nullptr;
    }
}

Renderpass* VulkanContext::addRenderpass(const RenderpassConfig& config) {
    if (renderpasses.getContext(config.name).index != INVALID_ID_U32) {
        Logger::logError("A renderpass with the name: " + config.name + " already exists!");
        return nullptr;
    }

    AssetContext context{};
    Renderpass& renderpass = *renderpasses.createAsset(config.name, context);

    renderpass.setId(context.index);
    renderpass.setClearFlags(config.clearFlags);
    renderpass.setClearColor(config.clearColor);
    renderpass.setRenderArea(config.renderArea);

    return &renderpass;
}

Renderpass *VulkanContext::getRenderpass(const unsigned char id) {
    return renderpasses.getAssetAtIndex(id);
}

Renderpass * VulkanContext::getRenderpass(const String &renderName) {
    return renderpasses.getAsset(renderName);
}

void VulkanContext::destroyContext() {
    geometries.shutdown();
    renderpasses.shutdown();

    resizeRenderTargetsEvent.destroyEvent();

    FF_Memory::ff_clear(&device.getSwapChainSupportInfo().capabilities, sizeof(device.getSwapChainSupportInfo().capabilities));

    device.getGraphicsQueueIndex() = -1;
    device.getPresentQueueIndex() = -1;
    device.getTransferQueueIndex() = -1;

    if (device.getCommandPool()) {
        Logger::logDebug("Destroying command pools.");
        vkDestroyCommandPool(device.getLogicalDevice(), device.getCommandPool(), allocator);
        device.getCommandPool() = nullptr;
    }

    Logger::logDebug("Destroying logical device.");
    if (device.getLogicalDevice()) {
        vkDestroyDevice(device.getLogicalDevice(), allocator);
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
        vkDestroySurfaceKHR(instance, surface, allocator);
        surface = nullptr;
    }

    if (instance) {
        Logger::logDebug("Destroying Vulkan instance.");
        vkDestroyInstance(instance, allocator);
        instance = nullptr;
    }

    if (allocator) {
        FF_Memory::ff_free(allocator, sizeof(VkAllocationCallbacks), RENDER, alignof(VkAllocationCallbacks));
        allocator = nullptr;
    }
}
