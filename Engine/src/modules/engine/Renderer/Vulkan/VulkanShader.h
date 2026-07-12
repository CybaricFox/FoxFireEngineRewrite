//
// Created by cmorg on 7/9/2026.
//

#pragma once
#include <vulkan/vulkan_core.h>

#include "src/modules/engine/Renderer/IRenderSystem.h"

struct VulkanShaderStage {
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
};

struct VulkanPipeline {
    VkPipeline handle;
    VkPipelineLayout pipelineLayout;
};

constexpr int STAGE_COUNT = 2;

class VulkanShader {
private:
    VulkanShaderStage stages[STAGE_COUNT]{};
    VulkanPipeline pipeline{};
public:
    VulkanShader();
    ~VulkanShader();

    void use();
};
