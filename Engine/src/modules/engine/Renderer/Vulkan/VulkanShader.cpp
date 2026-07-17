//
// Created by cmorg on 7/9/2026.
//

#include "VulkanShader.h"

#include "VulkanUtils.h"
#include "src/defines.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Library/FileHandler.h"

#define FF_OBJECT_SHADER "Vulkan_Object_Shader"

bool VulkanShader::createShaderModule(VulkanContext &context, const String &name, const String &typeStr, VkShaderStageFlagBits stageFlags, unsigned int stageIndex) {
    String fileName = "Shaders/" + name + "." + typeStr + ".spv";

    FF_Memory::ff_clear(&stages[stageIndex].createInfo, sizeof(VkShaderModuleCreateInfo));
    stages[stageIndex].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    FileHandler fileHandler{};
    if (!fileHandler.openFile(fileName, READ, true)) {
        Logger::logError("Can't open shader module file: " + name);
        return false;
    }

    unsigned long size = 0;
    unsigned char* buffer = nullptr;
    if (!fileHandler.readAll(buffer, size)) {
        Logger::logError("Unable to read binary shader module file: " + name);
        return false;
    }

    stages[stageIndex].createInfo.codeSize = size;
    stages[stageIndex].createInfo.pCode = reinterpret_cast<unsigned int*>(buffer);

    fileHandler.closeFile();

    VulkanUtils::vulkanCheck(vkCreateShaderModule(context.getDevice().getLogicalDevice(), &stages[stageIndex].createInfo, nullptr, &stages[stageIndex].handle));

    FF_Memory::ff_clear(&stages[stageIndex].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    stages[stageIndex].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[stageIndex].shaderStageCreateInfo.stage = stageFlags;
    stages[stageIndex].shaderStageCreateInfo.module = stages[stageIndex].handle;
    stages[stageIndex].shaderStageCreateInfo.pName = "main";

    if (buffer) {
        FF_Memory::ff_free(buffer, sizeof(unsigned char) * size, CHAR_ARRAY);
        buffer = nullptr;
    }

    return true;
}

bool VulkanShader::initialize(VulkanContext &context) {
    const String stageTypeStrings[STAGE_COUNT] = {"vert", "frag"};
    constexpr VkShaderStageFlagBits stageTypes[STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (unsigned int i = 0; i < STAGE_COUNT; i++) {
        if (!createShaderModule(context, FF_OBJECT_SHADER, stageTypeStrings[i], stageTypes[i], i)) {
            Logger::logError("Unable to create " + stageTypeStrings[i] + " for " + FF_OBJECT_SHADER);
            return false;
        }
    }

    //Initialize descriptors
    //HERE

    //Create pipeline
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = static_cast<float>(context.getFrameBufferHeight());
    viewport.width = static_cast<float>(context.getFrameBufferWidth());
    viewport.height = static_cast<float>(context.getFrameBufferHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context.getFrameBufferWidth();
    scissor.extent.height = context.getFrameBufferHeight();

    unsigned int offset = 0;
    constexpr int attributeCount = 1; //Attribute count is equal to the number of fields in Vertex3d.
    VkVertexInputAttributeDescription attributeDescriptions[attributeCount];

    constexpr VkFormat formats[attributeCount] = {VK_FORMAT_R32G32B32_SFLOAT};
    constexpr unsigned long sizes[attributeCount] = {sizeof(Vector3f)};

    for (unsigned int i = 0; i < attributeCount; i++) {
        attributeDescriptions[i].binding = 0;
        attributeDescriptions[i].location = i;
        attributeDescriptions[i].format = formats[i];
        attributeDescriptions[i].offset = offset;
        offset += sizes[i];
    }

    //Create stages
    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[STAGE_COUNT];
    FF_Memory::ff_clear(shaderStageCreateInfos, sizeof(shaderStageCreateInfos));

    for (unsigned int i = 0; i < STAGE_COUNT; i++) {
        shaderStageCreateInfos[i].sType = stages[i].shaderStageCreateInfo.sType;
        shaderStageCreateInfos[i] = stages[i].shaderStageCreateInfo;
    }

    if (!pipeline.createPipeline(context.getRenderpass(), attributeCount, attributeDescriptions, 0, nullptr, STAGE_COUNT, shaderStageCreateInfos, viewport, scissor, false,context.getDevice())) {
        Logger::logError("Failed to load graphics pipeline for object shader!");
        return false;
    }

    return true;
}

void VulkanShader::destroy(VulkanDevice& device) {
    pipeline.destroyPipeline(device);

    for (auto & stage : stages) {
        vkDestroyShaderModule(device.getLogicalDevice(), stage.handle, nullptr);
        stage.handle = nullptr;
    }
}

void VulkanShader::use() {

}
