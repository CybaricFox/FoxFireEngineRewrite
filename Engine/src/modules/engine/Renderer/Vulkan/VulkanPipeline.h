//
// Created by cmorg on 7/15/2026.
//

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
    void bindPipeline(const VulkanCommandBuffer& commandBuffer, VkPipelineBindPoint pipelineBindPoint) const;

    bool createPipeline(
        VulkanRenderpass &renderpass,
        unsigned int attributeCount,
        VkVertexInputAttributeDescription *attributes,
        unsigned int descriptorSetLayoutCount,
        VkDescriptorSetLayout *descriptorSetLayouts,
        unsigned int stageCount,
        VkPipelineShaderStageCreateInfo *shaderStages,
        VkViewport viewport,
        VkRect2D scissor,
        bool bIsWireframe, VulkanDevice &device
    );
};
