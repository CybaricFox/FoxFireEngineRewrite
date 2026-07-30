//
// Created by cmorg on 7/30/2026.
//

#include "VulkanFence.h"

#include "VulkanUtils.h"

void VulkanFence::createFence(VulkanDevice& device, const bool bCreateSignaled) {
    bIsSignaled = bCreateSignaled;
    VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (bIsSignaled) {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    VulkanUtils::vulkanCheck(vkCreateFence(device.getLogicalDevice(), &fenceCreateInfo, nullptr, &handle));
}

bool VulkanFence::waitForFence(VulkanDevice& device, const unsigned long timeout) {
    if (!bIsSignaled) {
        switch (vkWaitForFences(device.getLogicalDevice(), 1, &handle, true, timeout)) {
            case VK_SUCCESS:
                bIsSignaled = true;
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

void VulkanFence::resetFence(VulkanDevice& device) {
    if (bIsSignaled) {
        VulkanUtils::vulkanCheck(vkResetFences(device.getLogicalDevice(), 1, &handle));
        bIsSignaled = false;
    }
}

void VulkanFence::destroyFence(VulkanDevice& device) {
    if (handle) {
        vkDestroyFence(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }
    bIsSignaled = false;
}
