//
// Created by cmorg on 8/7/2026.
//

#include "VulkanBackendShader.h"

#include "VulkanUtils.h"

bool VulkanBackendShader::setStages(const unsigned char stageCount, DynamicArray<ShaderStage>& shaderStages, DynamicArray<String>& stageFileNames) {
    for (unsigned int i = 0; i < stageCount; i++) {
        if (config.stageCount + 1 > VULKAN_SHADER_MAX_STAGES) {
            Logger::logError("Too many shader stages. Cannot have more stages than " + std::to_string(VULKAN_SHADER_MAX_STAGES) + ".");
            return false;
        }

        VkShaderStageFlagBits stageFlag{};
        switch (shaderStages[i]) {
            case SHADER_STAGE_VERTEX: {
                stageFlag = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            }
            case SHADER_STAGE_FRAGMENT: {
                stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            }
            default: {
                Logger::logError("Unsupported shader stage: " + std::to_string(shaderStages[i]));
                continue;
            }
        }

        config.stages[config.stageCount].stage = stageFlag;
        config.stages[config.stageCount].fileName = stageFileNames[i];
        config.stageCount++;
    }

    return true;
}

void VulkanBackendShader::initializeDescriptorSets(const unsigned int imageCount) {
    descriptorSets.initialize(imageCount);
}

void VulkanBackendShader::setPoolSizes() {
    config.poolSizes[0] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024};
    config.poolSizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096};
}

void VulkanBackendShader::createUBOConfig(const unsigned int index, const unsigned int descriptorIndex) {
    VulkanDescriptorSetConfig newConfig{};

    newConfig.bindings[index].binding = index;
    newConfig.bindings[index].descriptorCount = 1;
    newConfig.bindings[index].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    newConfig.bindings[index].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    newConfig.bindingCount++;

    config.descriptorSets[descriptorIndex] = newConfig;
    config.descriptorSetCount++;
}

void VulkanBackendShader::shutdown(VulkanDevice& device, const VkAllocationCallbacks* allocator) {
    for (unsigned int i = 0; i < config.descriptorSetCount; i++) {
        if (descriptorSetLayouts[i]) {
            vkDestroyDescriptorSetLayout(device.getLogicalDevice(), descriptorSetLayouts[i], allocator);
            descriptorSetLayouts[i] = nullptr;
        }
    }

    if (descriptorPool) {
        vkDestroyDescriptorPool(device.getLogicalDevice(), descriptorPool, allocator);
    }

    uniformBuffer.unlockBuffer(device);
    uniformBuffer.destroyBuffer(device);

    pipeline.destroyPipeline(device);

    for (unsigned int i = 0; i < config.stageCount; i++) {
        vkDestroyShaderModule(device.getLogicalDevice(), stages[i].handle, allocator);
    }

    FF_Memory::ff_clear(&config, sizeof(VulkanShaderConfig));
}

void VulkanBackendShader::setAttribute(const unsigned int index, const VkFormat format, const unsigned int offset) {
    VkVertexInputAttributeDescription& attribute = config.attributes[index];
    attribute.location = index;
    attribute.binding = 0;
    attribute.offset = offset;
    attribute.format = format;
}

void VulkanBackendShader::setDescriptorSetConfig(const unsigned int index, const unsigned int samplerIndex) {
    VulkanDescriptorSetConfig& descriptorConfig = config.descriptorSets[index];

    if (descriptorConfig.bindingCount < 2) {
        descriptorConfig.bindings[samplerIndex].binding = samplerIndex;
        descriptorConfig.bindings[samplerIndex].descriptorCount = 1;
        descriptorConfig.bindings[samplerIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorConfig.bindings[samplerIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorConfig.bindingCount++;
    } else {
        descriptorConfig.bindings[samplerIndex].descriptorCount++;
    }
}

bool VulkanBackendShader::createDescriptorPool(VulkanDevice& device, const VkAllocationCallbacks* allocator) {
    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = config.poolSizes;
    poolInfo.maxSets = config.maxDescriptorCount;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkResult result = vkCreateDescriptorPool(device.getLogicalDevice(), &poolInfo, allocator, &descriptorPool);
    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("Failed to create descriptor pool for vulkan shader. " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    return true;
}

bool VulkanBackendShader::createDescriptorSetLayout(const unsigned int index, VulkanDevice& device, const VkAllocationCallbacks* allocator) {
    VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = config.descriptorSets[index].bindingCount;
    layoutInfo.pBindings = config.descriptorSets[index].bindings;
    VkResult result = vkCreateDescriptorSetLayout(device.getLogicalDevice(), &layoutInfo, allocator, &descriptorSetLayouts[index]);
    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("Vulkan shader failed to create descriptor set layout. " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    return true;
}

void VulkanBackendShader::finalizeBuffer(VulkanDevice& device) {
    uniformBufferMemoryBlock = uniformBuffer.lockBufferWhole(device, 0, 0);
}

void VulkanBackendShader::finalizeDescriptorSets(const unsigned int imagecount, const unsigned int descriptorIndex, VulkanDevice& device) {
    DynamicArray<VkDescriptorSetLayout> globalLayouts(imagecount);
    for (unsigned int i = 0; i < imagecount; i++) {
        globalLayouts.push(descriptorSetLayouts[descriptorIndex]);
    }

    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = imagecount;
    allocInfo.pSetLayouts = globalLayouts.getData();
    VulkanUtils::vulkanCheck(vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, descriptorSets.getData()));
}

bool VulkanBackendShader::createPipeline(const unsigned int stride, const unsigned int attributeCount,
                                         VkPipelineShaderStageCreateInfo* shaderStageCreateInfos, VkViewport viewport, VkRect2D scissor,
                                         const unsigned int pushConstantRangeCount, MemoryRange* memoryRanges, VulkanDevice& device) {

    if (!renderpass) {
        Logger::logFatal("Attempted to create pipeline for a shader but renderpass is null!");
        return false;
    }

    return pipeline.createPipeline(
        *renderpass,
        stride,
        attributeCount,
        config.attributes,
        config.descriptorSetCount,
        descriptorSetLayouts,
        config.stageCount,
        shaderStageCreateInfos,
        viewport,
        scissor,
        false,
        true,
        pushConstantRangeCount,
        memoryRanges,
        device);
}
