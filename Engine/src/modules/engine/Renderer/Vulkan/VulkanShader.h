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
    bool createBuffer(VulkanContext& context, unsigned long size, VkBufferUsageFlagBits usage, unsigned int memoryFlags, bool bBind, VulkanBuffer& buffer);
    void destroyBuffer(VulkanDevice &device, VulkanBuffer &buffer);
    void* lockBuffer(VulkanDevice &device, const VulkanBuffer &buffer, unsigned long offset, unsigned long size, unsigned int flags);
    void unlockBuffer(VulkanDevice &device, const VulkanBuffer &buffer);
    void loadBufferData(VulkanDevice &device, const VulkanBuffer &buffer, unsigned long offset, unsigned long size, unsigned int flags, const void *
                        data);
    void copyBufferData(VulkanContext& context, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, unsigned long sourceOffset, VkBuffer dest, unsigned long destOffset, unsigned long size);
    bool resizeBuffer(VulkanContext &context, unsigned long newSize, VulkanBuffer &buffer, VkQueue queue, VkCommandPool pool);
    void bindBuffer(VulkanDevice &device, const VulkanBuffer &buffer, unsigned long offset);

public:
    bool initialize(VulkanContext& context);
    void destroy(VulkanContext &context);
    void use();
    bool createBuffers(VulkanContext& context);
};
