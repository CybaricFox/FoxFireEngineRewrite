/**
*   @file VulkanPipeline.h
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
#include "VulkanSwapchain.h"


struct VulkanPipelineConfig {
    VulkanRenderpass* renderpass = nullptr;
    unsigned int stride = 0;
    unsigned int attributeCount = 0;
    VkVertexInputAttributeDescription* attributes = nullptr;
    unsigned int descriptorSetLayoutCount = 0;
    VkDescriptorSetLayout *descriptorSetLayouts = nullptr;
    unsigned int stageCount = 0;
    VkPipelineShaderStageCreateInfo *shaderStages = nullptr;
    VkViewport viewport{};
    VkRect2D scissor{};
    CullMode cullMode{};
    bool bIsWireframe = false;
    unsigned int shaderFlags{};
    unsigned int pushConstantRangeCount = 0;
    MemoryRange* pushConstantRanges = nullptr;
};

class VulkanPipeline {
private:
    VkPipeline handle{};
    VkPipelineLayout pipelineLayout{};

public:
    VkPipelineLayout& getPipelineLayout() {return pipelineLayout;}

    void destroyPipeline(VulkanDevice &device, const VkAllocationCallbacks *allocator);
    void bindPipeline(VulkanCommandBuffer &commandBuffer, VkPipelineBindPoint pipelineBindPoint) const;
    bool createPipeline(VulkanPipelineConfig& config, VulkanDevice &device, VkAllocationCallbacks *allocator);
};
