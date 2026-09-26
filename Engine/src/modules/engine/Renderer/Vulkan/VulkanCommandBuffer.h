/**
*   @file VulkanCommandBuffer.h
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

#include "VulkanDevice.h"
#include "VulkanState.h"

class VulkanCommandBuffer {
private:
    VkCommandBuffer handle{};
    VulkanState state{};

public:
    /**
     * @brief Gets the VKCommandBuffer.
     * @return
     */
    VkCommandBuffer& getHandle() {return handle;}
    /**
     * @brief Gets the current state of this command buffer.
     * @return
     */
    [[nodiscard]] VulkanState getState() const {return state;}

    /**
     * @brief Sets the command buffer's state.
     * @param newState
     */
    void setState(const VulkanState newState) {state = newState;}
    /**
     * @brief Destroys the VKCommandBuffer.
     */
    void destroyHandle() {handle = nullptr;}

    /**
     * @brief Allocates and begins a command buffer for single time use.
     * @param device
     * @return A single use command buffer.
     */
    static VulkanCommandBuffer allocateAndBeginSingleUseCommandBuffer(VulkanDevice &device);

    /**
     * @brief Ends a single use command buffer. Only call if a single use command buffer is currently being used.
     * @param queue
     * @param device
     */
    void endSingleUseCommandBuffer(VkQueue queue, VulkanDevice &device);

    /**
     * @brief Frees the command buffer.
     * @param device
     */
    void freeCommandBuffer(VulkanDevice &device);

    /**
     * @brief Allocates the command buffer.
     * @param bIsPrimary Whether this is the primary command buffer.
     * @param device
     */
    void allocateCommandBuffer(bool bIsPrimary, VulkanDevice& device);

    /**
     * @brief Sets the state to ready.
     */
    void resetCommandBuffer();

    /**
     * @brief Begins the command buffer.
     * @param bIsSingleUse Whether this is a single use command buffer.
     * @param bIsRenderpassContinue
     * @param bIsConcurrent
     */
    void beginCommandBuffer(bool bIsSingleUse, bool bIsRenderpassContinue, bool bIsConcurrent);

    /**
     * @brief Ends the command buffer.
     */
    void endCommandBuffer();

    /**
     * @brief Sets the state to Submitted.
     */
    void updateSubmittedCommandBuffer();
};
