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
    unsigned int generations[3];
};

constexpr int STAGE_COUNT = 2;
constexpr int MAX_ENTITIES = 1024;
constexpr int DESCRIPTOR_COUNT = 2;

struct EntityState {
    VkDescriptorSet descriptorSets[3]{}; //per frame
    VulkanDescriptorState descriptorStates[DESCRIPTOR_COUNT]{};//per entity
};

class VulkanShader {
private:
    VulkanShaderStage stages[STAGE_COUNT]{};
    VulkanPipeline pipeline{};
    GlobalUniform globalUBO{};
    VkDescriptorPool globalDescriptorPool{};
    //1 per frame. Max 3 for triple buffering (Mailbox).
    VkDescriptorSet globalDescriptorSets[3]{};
    VkDescriptorSetLayout globalDescriptorSetLayout{};
    VulkanBuffer globalUniformBuffer{};
    VkDescriptorPool entityDescriptorPool{};
    VkDescriptorSetLayout entityDescriptorLayout{};
    VulkanBuffer entityUniformBuffer{};
    unsigned int entityUniformBufferIndex = 0;
    EntityState entityStates[MAX_ENTITIES]{};

    //Creates a shader module from a .spv file. Name is the name of the file and TypeStr is the suffix (frag, vert). Do not include '.' in the typeStr!
    bool createShaderModule(VulkanContext& context, const String &name, const String& typeStr, VkShaderStageFlagBits stageFlags, unsigned int stageIndex);
    void* lockBuffer(VulkanDevice &device, const VulkanBuffer &buffer, unsigned long offset, unsigned long size, unsigned int flags);
    void unlockBuffer(VulkanDevice &device, const VulkanBuffer &buffer);
    bool resizeBuffer(VulkanContext &context, unsigned long newSize, VulkanBuffer &buffer, VkQueue queue, VkCommandPool pool);
    void bindBuffer(VulkanDevice &device, const VulkanBuffer &buffer, unsigned long offset);

public:
    GlobalUniform& getUBO() {return globalUBO;}

    bool initialize(VulkanContext& context);
    void destroy(VulkanContext &context);
    void use(VulkanContext &context) const;
    void updateGlobalState(VulkanContext &context);
    bool createBuffers(VulkanContext& context);
    bool createBuffer(VulkanContext& context, unsigned long size, VkBufferUsageFlagBits usage, unsigned int memoryFlags, bool bBind, VulkanBuffer& buffer);
    void loadBufferData(VulkanDevice &device, const VulkanBuffer &buffer, unsigned long offset, unsigned long size, const void *data);
    void copyBufferData(VulkanContext& context, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, unsigned long sourceOffset, VkBuffer dest, unsigned long destOffset, unsigned long size);
    void destroyBuffer(VulkanDevice &device, VulkanBuffer &buffer);
    void updateEntity(VulkanContext &context, const GeometryRenderData &data);
    bool aquireResources(VulkanContext& context, unsigned int& outId);
    void releaseResources(VulkanContext& context, unsigned int id);
};
