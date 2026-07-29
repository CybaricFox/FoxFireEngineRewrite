//
// Created by cmorg on 7/4/2026.
//

#pragma once
#include <vulkan/vulkan.h>
#include "src/defines.h"


class VulkanUtils {
public:
    static String getResultAsString(VkResult result, bool bGetExtended);

    static bool vulkanCheck(VkResult result);

    static int findMemoryIndex(int typeFilter, unsigned int propertyFlags, VkPhysicalDevice physicalDevice);
};
