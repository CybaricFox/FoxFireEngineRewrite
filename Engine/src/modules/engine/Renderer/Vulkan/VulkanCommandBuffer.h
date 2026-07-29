//
// Created by cmorg on 7/28/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanState.h"

struct VulkanCommandBuffer {
    VkCommandBuffer handle;
    VulkanState state;
};
