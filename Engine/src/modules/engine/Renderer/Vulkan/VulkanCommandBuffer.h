//
// Created by cmorg on 7/28/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanDevice.h"
#include "VulkanState.h"

class VulkanCommandBuffer {
private:
    VkCommandBuffer handle{};
    VulkanState state{};

public:
    VkCommandBuffer& getHandle() {return handle;}
    [[nodiscard]] VulkanState getState() const {return state;}

    void setState(const VulkanState newState) {state = newState;}
    void destroyHandle() {handle = nullptr;}

    static VulkanCommandBuffer allocateAndBeginSingleUseCommandBuffer(VulkanDevice &device);

    void endSingleUseCommandBuffer(VkQueue queue, VulkanDevice &device);
    void freeCommandBuffer(VulkanDevice &device);
    void allocateCommandBuffer(bool bIsPrimary, VulkanDevice& device);
    void resetCommandBuffer();
    void beginCommandBuffer(bool bIsSingleUse, bool bIsRenderpassContinue, bool bIsConcurrent);
    void endCommandBuffer();
    void updateSubmittedCommandBuffer();
};
