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
    unsigned char globalUniformCount = 0;
    unsigned char globalUniformSamplerCount = 0;
    unsigned char instanceUniformCount = 0;
    unsigned char instanceUniformSamplerCount = 0;
    unsigned char localUniformCount = 0;

public:
    /**
     * @brief Gets the number of stages.
     * @return
     */
    [[nodiscard]] unsigned char getStageCount() const {return config.stageCount;}
    /**
     * @brief Gets the stage at the given index.
     * @param index
     * @return
     */
    VulkanShaderStage& getStage(const unsigned char index) {return stages[index];}
    /**
     * @brief Gets the shader config.
     * @return
     */
    VulkanShaderConfig& getConfig() {return config;}
    /**
     * @brief Gets the uniform buffer.
     * @return
     */
    VulkanBuffer& getUniformBuffer() {return uniformBuffer;}
    /**
     * @brief Gets the pipeline used by this shader.
     * @return
     */
    VulkanPipeline& getPipeline() {return pipeline;}
    /**
     * @brief Gets the instance state with the given id.
     * @param instanceId
     * @return
     */
    VulkanShaderInstanceState& getInstanceState(const unsigned int instanceId) {return instanceStates[instanceId];}
    /**
     * @brief Gets the descriptor set at the given index.
     * @param index
     * @return
     */
    VkDescriptorSet& getDescriptorSet(const unsigned char index) {return descriptorSets[index];}
    /**
     * @brief Gets the config for the descriptor set at the index.
     * @param index
     * @return
     */
    VulkanDescriptorSetConfig& getDescriptorSetConfig(const unsigned int index) {return config.descriptorSets[index];}
    /**
     * @brief Gets the descriptor set layout for the set at the index.
     * @param index
     * @return
     */
    VkDescriptorSetLayout& getDescriptorSetLayout(const unsigned char index) {return descriptorSetLayouts[index];}
    /**
     * @brief Gets the descriptor pool.
     * @return
     */
    VkDescriptorPool& getDescriptorPool() {return descriptorPool;}
    /**
     * @brief Returns a pointer to the uniform buffers memory.
     * @return
     */
    [[nodiscard]] void* getUniformBufferMemoryBlock() const {return uniformBufferMemoryBlock;}
    /**
     * @brief Gets the number of global uniforms.
     * @return
     */
    [[nodiscard]] unsigned char getGlobalUniformCount() const {return globalUniformCount;}
    /**
     * @brief Gets the number of global samplers.
     * @return
     */
    [[nodiscard]] unsigned char getGlobalSamplerCount() const {return globalUniformSamplerCount;}
    /**
     * @brief Gets the number of instance uniforms.
     * @return
     */
    [[nodiscard]] unsigned char getInstanceUniformCount() const {return instanceUniformCount;}
    /**
     * @brief Gets the number of instance samplers.
     * @return
     */
    [[nodiscard]] unsigned char getInstanceSamplerCount() const {return instanceUniformSamplerCount;}
    /**
     * @brief Gets the number of local uniforms.
     * @return
     */
    [[nodiscard]] unsigned char getLocalUniformCount() const {return localUniformCount;}

    /**
     * @brief Set the renderpass this shader will use.
     * @param newRenderpass
     */
    void setRenderpass(VulkanRenderpass &newRenderpass) {renderpass = &newRenderpass;}
    /**
     * @brief Set the maximum number of descriptors.
     * @param maxDescriptorCount
     */
    void setMaxDescriptorCount(const unsigned int maxDescriptorCount) {config.maxDescriptorCount = maxDescriptorCount;}
    /**
     * @brief Increment the stage count.
     */
    void incrementStageCount() {config.stageCount++;}

    /**
     * @brief Increment the sampler count.
     * @param scope Which sampler type to increment (Only GLOBAL or INSTANCE).
     */
    void incrementSamplerCount(ShaderScope scope);

    /**
     * @brief Increment the uniform count.
     * @param scope Which sampler type to increment.
     */
    void incrementUniformCount(ShaderScope scope);

    /**
     * @brief Set the stages.
     * @param stageCount Number of stages.
     * @param shaderStages Dynamic array of stages.
     * @param stageFileNames Dynamic array of files.
     * @return True on success.
     */
    bool setStages(unsigned char stageCount, DynamicArray<ShaderStage> &shaderStages, DynamicArray<String> &stageFileNames);

    /**
     * @brief Initializes the descriptor sets.
     * @param imageCount Number of swapchain images.
     */
    void initializeDescriptorSets(unsigned int imageCount);

    /**
     * @brief Set the pool sizes. Currently hard coded.
     */
    void setPoolSizes();

    /**
     * @brief Creates the global UBO config.
     * @param index Unifrom index.
     * @param descriptorIndex Index of the associated descriptor set.
     */
    void createUBOConfig(unsigned int index, unsigned int descriptorIndex);
    void shutdown(VulkanDevice &device, const VkAllocationCallbacks *allocator);

    /**
     * @brief Sets an attribute.
     * @param index Index of the attribute.
     * @param format
     * @param offset
     */
    void setAttribute(unsigned int index, VkFormat format, unsigned int offset);

    /**
     * @brief Sets the config for a descriptor set.
     * @param index Index of the descriptor set.
     * @param samplerIndex Index of an associated sampler.
     */
    void setDescriptorSetConfig(unsigned int index, unsigned int samplerIndex);

    /**
     * @brief Creates a descriptor pool.
     * @param device
     * @param allocator
     * @return
     */
    bool createDescriptorPool(VulkanDevice &device, const VkAllocationCallbacks *allocator);

    /**
     * @brief Creates a descpriptor set layout.
     * @param index Index of the descriptor set.
     * @param device
     * @param allocator
     * @return
     */
    bool createDescriptorSetLayout(unsigned int index, VulkanDevice &device, const VkAllocationCallbacks *allocator);

    /**
     * @brief Locks the uniform's buffer.
     * @param device
     */
    void finalizeBuffer(VulkanDevice &device);

    /**
     * @brief Tells vulkan to allocate the descriptor sets.
     * @param imagecount Number of swapchain images.
     * @param descriptorIndex Index of the descriptor set.
     * @param device
     */
    void finalizeDescriptorSets(unsigned int imagecount, unsigned int descriptorIndex, VulkanDevice &device);

    /**
     * @brief Creates the pipeline for this shader.
     * @param stride
     * @param attributeCount
     * @param shaderStageCreateInfos
     * @param viewport
     * @param scissor
     * @param pushConstantRangeCount
     * @param memoryRanges
     * @param device
     * @return
     */
    bool createPipeline(unsigned int stride, unsigned int attributeCount,
                        VkPipelineShaderStageCreateInfo *shaderStageCreateInfos, VkViewport viewport, VkRect2D scissor,
                        unsigned int pushConstantRangeCount, MemoryRange *memoryRanges, VulkanDevice &device);
};