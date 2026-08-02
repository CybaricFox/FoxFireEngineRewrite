//
// Created by cmorg on 7/9/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "src/modules/engine/Renderer/GlobalUniform.h"

struct VulkanShaderStage {
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
};

struct VulkanDescriptorState {
    DynamicArray<unsigned int> generations{};
    DynamicArray<unsigned int> ids{};
};

constexpr int STAGE_COUNT = 2;
constexpr int INITIAL_MATERIALS = 1024;
constexpr int DESCRIPTOR_COUNT = 2;
constexpr int SAMPLER_COUNT = 1;

struct MaterialState {
    bool bIsValid = false;
    DynamicArray<VkDescriptorSet> descriptorSets{}; //per frame
    VulkanDescriptorState descriptorStates[DESCRIPTOR_COUNT]{};//per entity

    void initialize(const unsigned int frameCount) {
        if (bIsValid) return;
        descriptorSets.initialize(frameCount);
        descriptorStates[0].generations.initialize(frameCount);
        descriptorStates[0].ids.initialize(frameCount);
        descriptorStates[1].generations.initialize(frameCount);
        descriptorStates[1].ids.initialize(frameCount);
        for (unsigned int i = 0; i < frameCount; i++) {
            descriptorStates[0].generations.emplace(INVALID_ID);
            descriptorStates[0].ids.emplace(INVALID_ID);
            descriptorStates[1].generations.emplace(INVALID_ID);
            descriptorStates[1].ids.emplace(INVALID_ID);
        }
        bIsValid = true;
    }
};

class VulkanShader {
private:
    VulkanShaderStage stages[STAGE_COUNT]{};
    VulkanPipeline pipeline{};
    GlobalUniform globalUBO{};
    VkDescriptorPool globalDescriptorPool{};
    //1 per frame. Max 3 for triple buffering (Mailbox).
    DynamicArray<VkDescriptorSet> globalDescriptorSets{};
    VkDescriptorSetLayout globalDescriptorSetLayout{};
    VulkanBuffer globalUniformBuffer{};
    VkDescriptorPool entityDescriptorPool{};
    VkDescriptorSetLayout entityDescriptorLayout{};
    VulkanBuffer entityUniformBuffer{};
    unsigned int entityUniformBufferIndex = 0;
    DynamicArray<MaterialState> materialStates{};
    TextureUseCase samplerUses[SAMPLER_COUNT]{};

    //Creates a shader module from a .spv file. Name is the name of the file and TypeStr is the suffix (frag, vert). Do not include '.' in the typeStr!
    bool createShaderModule(VulkanContext& context, const String &name, const String& typeStr, VkShaderStageFlagBits stageFlags, unsigned int stageIndex);

public:
    GlobalUniform& getUBO() {return globalUBO;}

    bool initialize(VulkanContext& context);
    void destroy(VulkanContext &context);
    void use(const VulkanContext &context) const;
    void updateGlobalState(VulkanContext &context);
    bool createBuffers(VulkanContext& context);
    void setModel(VulkanContext& context, const Mat4 &model);
    void applyMaterial(VulkanContext& context, Material& material, Texture& defaultTexture);
    bool aquireResources(VulkanContext& context, Material& material);
    void releaseResources(VulkanContext &context, Material &material);
};
