//
// Created by cmorg on 7/3/2026.
//

#include "VulkanContext.h"

#include "src/modules/engine/Library/FF_Memory.h"
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

void VulkanContext::destroyContext() {
    Logger::logInfo("Releasing Vulkan device resources");

    FF_Memory::ff_clear(&device.swapChainSupportInfo.capabilities, sizeof(device.swapChainSupportInfo.capabilities));

    device.graphicsQueueIndex = -1;
    device.presentQueueIndex = -1;
    device.transferQueueIndex = -1;

    if (device.logicalDevice) {
        vkDestroyDevice(device.logicalDevice, nullptr);
        device.logicalDevice = nullptr;
    }

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
