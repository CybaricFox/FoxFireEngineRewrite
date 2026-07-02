//
// Created by cmorg on 7/2/2026.
//

#pragma once
#include "RendererBackend.h"
#include <vulkan/vulkan.h>

#include "src/modules/engine/Core/GameInstance.h"

enum VulkanTypes {

};

struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks allocator;
};

class VulkanBackend final : public RendererBackend{
private:
    int majorVersion = 0;
    int minorVersion = 0;
    int patchVersion = 0;

public:
    static VulkanContext vulkanContext;

    bool initialize(String appName, PlatformState *newPlatformState) override;

    void setVersion(const GameInstance* gameInstance);
};
