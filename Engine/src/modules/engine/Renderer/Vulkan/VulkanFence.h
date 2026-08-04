//
// Created by cmorg on 7/30/2026.
//

#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"


class FOXFIRE_DEPRECATED VulkanFence {
private:
    VkFence handle{};
    bool bIsSignaled = false;

public:
    VkFence getFence() const {return handle;}

    void createFence(VulkanDevice& device, bool bCreateSignaled);
    bool waitForFence(VulkanDevice &device, unsigned long timeout);
    void resetFence(VulkanDevice &device);
    void destroyFence(VulkanDevice &device);
};
