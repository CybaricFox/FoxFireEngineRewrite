//
// Created by cmorg on 8/7/2026.
//

#pragma once
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderpass.h"
#include "VulkanTypes.h"
#include "src/defines.h"
#include "src/modules/engine/Renderer/IBackendShader.h"

/**
 *  @file VulkanBackendShader.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/7/2026
 *
 *  @copyright (c) 2026
 */

class VulkanBackendShader : public IBackendShader{
private:
    unsigned int id = INVALID_ID_U32;
    VulkanShaderConfig config{};
    VulkanRenderpass* renderpass = nullptr;
    VulkanShaderStage stages[VULKAN_SHADER_MAX_STAGES]{};
    VkDescriptorPool descriptorPool{};
    VkDescriptorSetLayout descriptorSetLayouts[2]{};
    DynamicArray<VkDescriptorSet> descriptorSets{};
    void* uniformBufferMemoryBlock = nullptr;
    VulkanBuffer uniformBuffer{};
    VulkanPipeline pipeline{};
    unsigned int instanceCount = 0;
    VulkanShaderInstanceState instanceStates[MAX_MATERIAL_COUNT]{};

public:
    [[nodiscard]] unsigned char getStageCount() const {return config.stageCount;}
    VulkanShaderStage& getStage(const unsigned char index) {return stages[index];}
    VulkanShaderConfig& getConfig() {return config;}
    VulkanBuffer& getUniformBuffer() {return uniformBuffer;}
    VulkanPipeline& getPipeline() {return pipeline;}
    VulkanShaderInstanceState& getInstanceState(const unsigned int instanceId) {return instanceStates[instanceId];}
    VkDescriptorSet& getDescriptorSet(const unsigned char index) {return descriptorSets[index];}
    VkDescriptorSetLayout& getDescriptorSetLayout(const unsigned char index) {return descriptorSetLayouts[index];}
    VkDescriptorPool& getDescriptorPool() {return descriptorPool;}
    [[nodiscard]] void* getUniformBufferMemoryBlock() const {return uniformBufferMemoryBlock;}

    void setRenderpass(VulkanRenderpass &newRenderpass) {renderpass = &newRenderpass;}
    void setMaxDescriptorCount(const unsigned int maxDescriptorCount) {config.maxDescriptorCount = maxDescriptorCount;}
    void incrementStageCount() {config.stageCount++;}

    bool setStages(unsigned char stageCount, DynamicArray<ShaderStage> &shaderStages, DynamicArray<String> &stageFileNames);
    void initializeDescriptorSets(unsigned int imageCount);
    void setPoolSizes();
    void createUBOConfig(unsigned int index, unsigned int descriptorIndex);
    void shutdown(VulkanDevice &device, const VkAllocationCallbacks *allocator);
    void setAttribute(unsigned int index, VkFormat format, unsigned int offset);
    void setDescriptorSetConfig(unsigned int index, unsigned int samplerIndex);
    bool createDescriptorPool(VulkanDevice &device, const VkAllocationCallbacks *allocator);
    bool createDescriptorSetLayout(unsigned int index, VulkanDevice &device, const VkAllocationCallbacks *allocator);
    void finalizeBuffer(VulkanDevice &device);
    void finalizeDescriptorSets(unsigned int imagecount, unsigned int descriptorIndex, VulkanDevice &device);

    bool createPipeline(unsigned int stride, unsigned int attributeCount,
        VkPipelineShaderStageCreateInfo *shaderStageCreateInfos, VkViewport viewport, VkRect2D scissor,
        unsigned int pushConstantRangeCount, MemoryRange *memoryRanges, VulkanDevice &device);
};