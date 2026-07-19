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

bool VulkanShader::createBuffer(VulkanContext &context, const unsigned long size, VkBufferUsageFlagBits usage, const unsigned int memoryFlags, const bool bBind, VulkanBuffer &buffer) {
    FF_Memory::ff_clear(&buffer, sizeof(VulkanBuffer));
    buffer.totalSize = size;
    buffer.usageFlags = usage;
    buffer.memoryPropertyFlags = memoryFlags;

    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VulkanUtils::vulkanCheck(vkCreateBuffer(context.getDevice().getLogicalDevice(), &bufferInfo, nullptr, &buffer.handle));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.getDevice().getLogicalDevice(), buffer.handle, &memRequirements);
    buffer.memoryIndex = context.getSwapchain().findMemoryIndex(static_cast<int>(memRequirements.memoryTypeBits), buffer.memoryPropertyFlags, context.getDevice());
    if (buffer.memoryIndex == -1) {
        Logger::logError("Can't find memory index for vulkan buffer!");
        return false;
    }

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = buffer.memoryIndex;

    VkResult result = vkAllocateMemory(context.getDevice().getLogicalDevice(), &allocInfo, nullptr, &buffer.deviceMemory);
    if (result != VK_SUCCESS) {
        Logger::logError("Failed to allocate memory for a new vulkan buffer: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    if (bBind) {
        bindBuffer(context.getDevice(), buffer, 0);
    }

    return true;
}

void VulkanShader::destroyBuffer(VulkanDevice& device, VulkanBuffer &buffer) {
    if (buffer.deviceMemory) {
        vkFreeMemory(device.getLogicalDevice(), buffer.deviceMemory, nullptr);
        buffer.deviceMemory = nullptr;
    }
    if (buffer.handle) {
        vkDestroyBuffer(device.getLogicalDevice(), buffer.handle, nullptr);
        buffer.handle = nullptr;
    }
    buffer.totalSize = 0;
    buffer.bIsLocked = false;
}

void * VulkanShader::lockBuffer(VulkanDevice& device, const VulkanBuffer &buffer, const unsigned long offset, const unsigned long size, const unsigned int flags) {
    void* data;
    VulkanUtils::vulkanCheck(vkMapMemory(device.getLogicalDevice(), buffer.deviceMemory, offset, size, flags, &data));
    return data;
}

void VulkanShader::unlockBuffer(VulkanDevice &device, const VulkanBuffer &buffer) {
    vkUnmapMemory(device.getLogicalDevice(), buffer.deviceMemory);
}

void VulkanShader::loadBufferData(VulkanDevice &device, const VulkanBuffer &buffer, const unsigned long offset, const unsigned long size, const void *data) {
    void* dataPtr;
    VulkanUtils::vulkanCheck(vkMapMemory(device.getLogicalDevice(), buffer.deviceMemory, offset, size, 0, &dataPtr));
    FF_Memory::ff_copy(dataPtr, data, size);
    vkUnmapMemory(device.getLogicalDevice(), buffer.deviceMemory);
}

void VulkanShader::copyBufferData(VulkanContext &context, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, unsigned long sourceOffset, VkBuffer dest, unsigned long destOffset, unsigned long size) {
    vkQueueWaitIdle(queue);

    //Create a one time use command buffer
    VulkanCommandBuffer tempBuffer{};
    context.allocateAndBeginSingleUseCommandBuffer(tempBuffer);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = sourceOffset;
    copyRegion.dstOffset = destOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(tempBuffer.handle, source, dest, 1, &copyRegion);

    context.endSingleUseCommandBuffer(tempBuffer, queue);
}

bool VulkanShader::resizeBuffer(VulkanContext &context, const unsigned long newSize, VulkanBuffer &buffer, VkQueue queue, VkCommandPool pool) {
    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = newSize;
    bufferInfo.usage = buffer.usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer newBuffer;
    VulkanUtils::vulkanCheck(vkCreateBuffer(context.getDevice().getLogicalDevice(), &bufferInfo, nullptr, &newBuffer));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context.getDevice().getLogicalDevice(), newBuffer, &requirements);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = buffer.memoryIndex;

    VkDeviceMemory newMemory;
    VkResult result = vkAllocateMemory(context.getDevice().getLogicalDevice(), &allocInfo, nullptr, &newMemory);
    if (result != VK_SUCCESS) {
        Logger::logError("Failed to resize vulkan buffer: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    VulkanUtils::vulkanCheck(vkBindBufferMemory(context.getDevice().getLogicalDevice(), newBuffer, newMemory, 0));

    copyBufferData(context, pool, nullptr, queue, buffer.handle, 0, newBuffer, 0, buffer.totalSize);

    vkDeviceWaitIdle(context.getDevice().getLogicalDevice());

    if (buffer.deviceMemory) {
        vkFreeMemory(context.getDevice().getLogicalDevice(), buffer.deviceMemory, nullptr);
        buffer.deviceMemory = nullptr;
    }
    if (buffer.handle) {
        vkDestroyBuffer(context.getDevice().getLogicalDevice(), buffer.handle, nullptr);
        buffer.handle = nullptr;
    }
    buffer.totalSize = newSize;
    buffer.deviceMemory = newMemory;
    buffer.handle = newBuffer;

    return true;
}

void VulkanShader::bindBuffer(VulkanDevice& device, const VulkanBuffer &buffer, const unsigned long offset) {
    VulkanUtils::vulkanCheck(vkBindBufferMemory(device.getLogicalDevice(), buffer.handle, buffer.deviceMemory, offset));
}

bool VulkanShader::createBuffers(VulkanContext &context) {
    VkMemoryPropertyFlags memoryPropertyFlags{VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    constexpr unsigned long vertexBufferSize = sizeof(Vertex3d) * 1024 * 1024; //Vertex Buffer should be 64mb with this
    if (!createBuffer(context, vertexBufferSize,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true, context.getVertexBuffer())) {

        Logger::logError("Error creating vertex buffer!");
        return false;
    }
    context.setVertexOffset(0);

    constexpr unsigned long indexBufferSize = sizeof(unsigned int) * 1024 * 1024;
    if (!createBuffer(context, indexBufferSize,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true, context.getIndexBuffer())) {

        Logger::logError("Error creating index buffer!");
        return false;
        }
    context.setIndexOffset(0);

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

    //Initialize global descriptors
    VkDescriptorSetLayoutBinding globalUBOLayoutBinding;
    globalUBOLayoutBinding.binding = 0;
    globalUBOLayoutBinding.descriptorCount = 1;
    globalUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalUBOLayoutBinding.pImmutableSamplers = nullptr;
    globalUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo globalLayoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    globalLayoutInfo.bindingCount = 1;
    globalLayoutInfo.pBindings = &globalUBOLayoutBinding;
    VulkanUtils::vulkanCheck(vkCreateDescriptorSetLayout(context.getDevice().getLogicalDevice(), &globalLayoutInfo, nullptr, &globalDescriptorSetLayout));

    VkDescriptorPoolSize globalPoolSize;
    globalPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalPoolSize.descriptorCount = context.getSwapchain().getImageCount();

    VkDescriptorPoolCreateInfo globalPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    globalPoolInfo.poolSizeCount = 1;
    globalPoolInfo.pPoolSizes = &globalPoolSize;
    globalPoolInfo.maxSets = context.getSwapchain().getImageCount();

    VulkanUtils::vulkanCheck(vkCreateDescriptorPool(context.getDevice().getLogicalDevice(), &globalPoolInfo, nullptr, &globalDescriptorPool));

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

    //Create descriptor set layouts
    constexpr int descriptorSetLayoutCount = 1;
    VkDescriptorSetLayout layouts[descriptorSetLayoutCount] = {globalDescriptorSetLayout};

    //Create stages
    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[STAGE_COUNT];
    FF_Memory::ff_clear(shaderStageCreateInfos, sizeof(shaderStageCreateInfos));

    for (unsigned int i = 0; i < STAGE_COUNT; i++) {
        shaderStageCreateInfos[i].sType = stages[i].shaderStageCreateInfo.sType;
        shaderStageCreateInfos[i] = stages[i].shaderStageCreateInfo;
    }

    if (!pipeline.createPipeline(context.getRenderpass(), attributeCount, attributeDescriptions, descriptorSetLayoutCount, layouts, STAGE_COUNT, shaderStageCreateInfos, viewport, scissor, false,context.getDevice())) {
        Logger::logError("Failed to load graphics pipeline for object shader!");
        return false;
    }

    if (!createBuffer(context, sizeof(GlobalUniform) * 3,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        true, globalUniformBuffer)) {

        Logger::logError("Failed to create global uniform buffer!");
        return false;
    }

    VkDescriptorSetLayout globalLayouts[3] = {globalDescriptorSetLayout, globalDescriptorSetLayout, globalDescriptorSetLayout};
    VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocateInfo.descriptorPool = globalDescriptorPool;
    allocateInfo.descriptorSetCount = 3;
    allocateInfo.pSetLayouts = globalLayouts;
    VulkanUtils::vulkanCheck(vkAllocateDescriptorSets(context.getDevice().getLogicalDevice(), &allocateInfo, globalDescriptorSets));

    return true;
}

void VulkanShader::destroy(VulkanContext& context) {
    destroyBuffer(context.getDevice(), context.getVertexBuffer());
    destroyBuffer(context.getDevice(), context.getIndexBuffer());

    destroyBuffer(context.getDevice(), globalUniformBuffer);

    pipeline.destroyPipeline(context.getDevice());

    vkDestroyDescriptorPool(context.getDevice().getLogicalDevice(), globalDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(context.getDevice().getLogicalDevice(), globalDescriptorSetLayout, nullptr);

    for (auto & stage : stages) {
        vkDestroyShaderModule(context.getDevice().getLogicalDevice(), stage.handle, nullptr);
        stage.handle = nullptr;
    }
}

void VulkanShader::use(VulkanContext& context) const {
    pipeline.bindPipeline(*context.getCommandBuffer(context.getImageIndex()), VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void VulkanShader::updateGlobalState(VulkanContext &context) {
    VkCommandBuffer commandBuffer = context.getCommandBuffer(context.getImageIndex())->handle;
    VkDescriptorSet globalDescriptorSet = globalDescriptorSets[context.getImageIndex()];

    constexpr unsigned int range = sizeof(GlobalUniform);
    const unsigned long offset = sizeof(GlobalUniform) * context.getImageIndex();

    loadBufferData(context.getDevice(), globalUniformBuffer, offset, range, &globalUBO);

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = globalUniformBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    VkWriteDescriptorSet writeDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writeDescriptorSet.dstSet = globalDescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context.getDevice().getLogicalDevice(), 1, &writeDescriptorSet, 0, nullptr);

    //This must be executed last because some GPUs cannot update a uniform buffer after binding it.
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayout(), 0, 1, &globalDescriptorSet, 0, nullptr);
}