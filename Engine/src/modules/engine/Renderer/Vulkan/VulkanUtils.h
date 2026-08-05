/**
*   @file VulkanUtils.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include <vulkan/vulkan.h>
#include "src/defines.h"

/**
 * @brief Contains a collection of useful functions for Vulkan.
 */
class VulkanUtils {
public:
    /**
     * @brief Converts a VulkanResult to a string
     * @param result The result
     * @param bGetExtended Whether to display extra information in the string.
     * @return String version of the result.
     */
    static String getResultAsString(VkResult result, bool bGetExtended);

    /**
     * @brief Checks that the result of a Vulkan Command is good.
     * @param result The result from the command.
     * @return True if good, false if bad.
     */
    static bool vulkanCheck(VkResult result);

    static int findMemoryIndex(int typeFilter, unsigned int propertyFlags, VkPhysicalDevice physicalDevice);
};
