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
#include "VulkanContext.h"
#include "VulkanSwapchain.h"


class VulkanPipeline {
private:
    VkPipeline handle{};
    VkPipelineLayout pipelineLayout{};

public:
    VkPipelineLayout& getPipelineLayout() {return pipelineLayout;}

    void destroyPipeline(VulkanDevice &device);
    void bindPipeline(VulkanCommandBuffer &commandBuffer, VkPipelineBindPoint pipelineBindPoint) const;

    bool createPipeline(
        VulkanRenderpass &renderpass,
        unsigned int stride,
        unsigned int attributeCount,
        VkVertexInputAttributeDescription *attributes,
        unsigned int descriptorSetLayoutCount,
        VkDescriptorSetLayout *descriptorSetLayouts,
        unsigned int stageCount,
        VkPipelineShaderStageCreateInfo *shaderStages,
        VkViewport viewport,
        VkRect2D scissor,
        bool bIsWireframe,
        bool bDepthTestEnabled,
        VulkanDevice &device
    );
};
