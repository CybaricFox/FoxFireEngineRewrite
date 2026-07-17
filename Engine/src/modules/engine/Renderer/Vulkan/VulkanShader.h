//
// Created by cmorg on 7/9/2026.
//

#pragma once
#include <vulkan/vulkan_core.h>

#include "VulkanContext.h"
#include "VulkanPipeline.h"

struct VulkanShaderStage {
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
};

constexpr int STAGE_COUNT = 2;

class VulkanShader {
private:
    VulkanShaderStage stages[STAGE_COUNT]{};
    VulkanPipeline pipeline{};

    //Creates a shader module from a .spv file. Name is the name of the file and TypeStr is the suffix (frag, vert). Do not include '.' in the typeStr!
    bool createShaderModule(VulkanContext& context, const String &name, const String& typeStr, VkShaderStageFlagBits stageFlags, unsigned int stageIndex);
public:
    bool initialize(VulkanContext& context);
    void destroy(VulkanDevice &device);
    void use();
};
