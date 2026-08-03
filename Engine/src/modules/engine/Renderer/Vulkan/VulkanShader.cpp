//
// Created by cmorg on 7/9/2026.
//

#include "VulkanShader.h"

#include "VulkanBackend.h"
#include "VulkanUtils.h"
#include "src/defines.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

#define FF_MATERIAL_SHADER "Vulkan_Material_Shader"

bool VulkanShader::createShaderModule(VulkanContext &context, const String &name, const String &typeStr, VkShaderStageFlagBits stageFlags, const unsigned int stageIndex, ResourceSystem& resources) {
    const String fileName = "../Shaders/" + name + "." + typeStr + ".spv";

    Resource binaryResource{};
    if (!resources.load(fileName, RESOURCE_TYPE_BINARY, binaryResource)) {
        Logger::logError("Unable to read shader module: " + fileName);
        return false;
    }

    FF_Memory::ff_clear(&stages[stageIndex].createInfo, sizeof(VkShaderModuleCreateInfo));
    stages[stageIndex].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    stages[stageIndex].createInfo.codeSize = binaryResource.dataSize;
    stages[stageIndex].createInfo.pCode = static_cast<unsigned int*>(binaryResource.data);

    VulkanUtils::vulkanCheck(vkCreateShaderModule(context.getDevice().getLogicalDevice(), &stages[stageIndex].createInfo, nullptr, &stages[stageIndex].handle));

    resources.unload(binaryResource);

    FF_Memory::ff_clear(&stages[stageIndex].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    stages[stageIndex].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[stageIndex].shaderStageCreateInfo.stage = stageFlags;
    stages[stageIndex].shaderStageCreateInfo.module = stages[stageIndex].handle;
    stages[stageIndex].shaderStageCreateInfo.pName = "main";

    return true;
}

bool VulkanShader::aquireResources(VulkanContext &context, Material &material) {
    material.internalId = entityUniformBufferIndex;
    entityUniformBufferIndex++;
    MaterialState* entityState = &materialStates[material.internalId];
    if (!entityState->bIsValid) entityState->initialize(context.getSwapchain().getImageCount());

    for (auto& descriptorState : entityState->descriptorStates) {
        for (unsigned int& generation : descriptorState.generations) {
            generation = INVALID_ID;
        }
        for (unsigned int& id : descriptorState.ids) {
            id = INVALID_ID;
        }
    }

    //allocate descriptor sets
    DynamicArray<VkDescriptorSetLayout> layouts{context.getSwapchain().getImageCount()};
    for (int i = 0; i < context.getSwapchain().getImageCount(); i++) {
        layouts.push(entityDescriptorLayout);
    }
    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = entityDescriptorPool;
    allocInfo.descriptorSetCount = context.getSwapchain().getImageCount();
    allocInfo.pSetLayouts = layouts.getData();

    VkResult result = vkAllocateDescriptorSets(context.getDevice().getLogicalDevice(), &allocInfo, entityState->descriptorSets.getData());
    if (result != VK_SUCCESS) {
        Logger::logError("Error allocating descriptor sets in shader!");
        return false;
    }

    return true;
}

void VulkanShader::releaseResources(VulkanContext &context, Material &material) {
    MaterialState* materialState = &materialStates[material.internalId];

    const unsigned int descriptorSetCount = context.getSwapchain().getImageCount();

    vkDeviceWaitIdle(context.getDevice().getLogicalDevice());

    VkResult result = vkFreeDescriptorSets(context.getDevice().getLogicalDevice(), entityDescriptorPool, descriptorSetCount, materialState->descriptorSets.getData());
    if (result != VK_SUCCESS) {
        Logger::logError("Error freeing descriptor sets in shader!");
    }

    for (auto& descriptorState : materialState->descriptorStates) {
        for (unsigned int& generation : descriptorState.generations) {
            generation = INVALID_ID;
        }
        for (unsigned int& idd : descriptorState.ids) {
            idd = INVALID_ID;
        }
    }

    material.internalId = INVALID_ID;
}

void VulkanShader::applyMaterial(VulkanContext &context, Material& material, Texture& defaultTexture) {
    unsigned int imageIndex = context.getImageIndex();
    VkCommandBuffer commandBuffer = context.getCurrentCommandBuffer().getHandle();

    //Obtain material data
    MaterialState* entityState = &materialStates[material.internalId];
    VkDescriptorSet entitySet = entityState->descriptorSets[context.getCurrentFrame()];
    VkWriteDescriptorSet descriptorWrites[DESCRIPTOR_COUNT];
    FF_Memory::ff_clear(descriptorWrites, sizeof(VkWriteDescriptorSet) * DESCRIPTOR_COUNT);
    unsigned int descriptorCount = 0;
    unsigned int descriptorIndex = 0;

    //Descriptor #0 - Uniform Buffer
    unsigned int range = sizeof(MaterialUniform);
    unsigned long offset = sizeof(MaterialUniform) * material.internalId;
    MaterialUniform obo{};

    //For Testing
    //static float accumulator = 0.0f;
    //accumulator += context.getDeltaTime();
    //const float s = (FF_Math::sin(accumulator) + 1) / 2; //Changes the scale form -1, 1 to 0, 1
    //obo.diffuse = createVector4f(s, s, s, 1);

    obo.diffuse = material.diffuseColor;
    entityUniformBuffer.loadBufferData(context.getDevice(), offset, range, &obo);

    unsigned int& uboGeneration = entityState->descriptorStates[descriptorIndex].generations[imageIndex];

    VkDescriptorBufferInfo bufferInfo{};
    VkWriteDescriptorSet descriptor{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    if (uboGeneration == INVALID_ID || uboGeneration != material.generation) {
        bufferInfo.buffer = entityUniformBuffer.getBuffer();
        bufferInfo.offset = offset;
        bufferInfo.range = range;

        descriptor.dstSet = entitySet;
        descriptor.dstBinding = descriptorIndex;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor.descriptorCount = 1;
        descriptor.pBufferInfo = &bufferInfo;

        descriptorWrites[descriptorCount] = descriptor;
        descriptorCount++;
        uboGeneration = material.generation;
    }

    descriptorIndex++;

    //Samplers
    constexpr unsigned int samplerCount = 1;
    VkDescriptorImageInfo imageInfos[1];
    VkWriteDescriptorSet descriptor1{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    for (unsigned int i = 0; i < samplerCount; i++) {
        TextureUseCase use = samplerUses[i];
        Texture* texture = nullptr;

        switch (use) {
            case TEXTURE_USE_MAP_DIFFUSE: {
                texture = material.diffuseMap.texture;
                break;
            }
            default: {
                Logger::logFatal("Texture use case is unknown. Sampler cannot be bound!");
                return;
            }
        }

        unsigned int& descriptorGeneration = entityState->descriptorStates[descriptorIndex].generations[context.getCurrentFrame()];
        unsigned int& descriptorID = entityState->descriptorStates[descriptorIndex].ids[context.getCurrentFrame()];

        //Prevents an unloaded texture from being loaded.
        if (texture->generation == INVALID_ID) {
            texture = &defaultTexture;
             descriptorGeneration = INVALID_ID;
        }

        if (texture && (descriptorID != texture->id || descriptorGeneration != texture->generation || descriptorGeneration == INVALID_ID)) {
            auto* internalData = static_cast<VulkanTextureData *>(texture->data);

            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[i].imageView = internalData->image.getImageView();
            imageInfos[i].sampler = internalData->sampler;

            descriptor1.dstSet = entitySet;
            descriptor1.dstBinding = descriptorIndex;
            descriptor1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor1.descriptorCount = 1;
            descriptor1.pImageInfo = &imageInfos[i];
            descriptorWrites[descriptorCount] = descriptor1;
            descriptorCount++;

            //Sync frame generation if the texture is not default
            if (texture->generation != INVALID_ID) {
                descriptorGeneration = texture->generation;
                descriptorID = texture->id;
            }
            descriptorIndex++;
        }
    }

    if (descriptorCount > 0) {
        vkUpdateDescriptorSets(context.getDevice().getLogicalDevice(), descriptorCount, descriptorWrites, 0, nullptr);
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayout(), 1, 1, &entitySet, 0, nullptr);
}

bool VulkanShader::createBuffers(VulkanContext &context) {
    VkMemoryPropertyFlags memoryPropertyFlags{VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    constexpr unsigned long vertexBufferSize = sizeof(Vertex3d) * 1024 * 1024; //Vertex Buffer should be 64mb with this
    if (!context.getVertexBuffer().createBuffer(context.getDevice(), vertexBufferSize,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true)) {

        Logger::logError("Error creating vertex buffer!");
        return false;
    }
    context.setVertexOffset(0);

    constexpr unsigned long indexBufferSize = sizeof(unsigned int) * 1024 * 1024;
    if (!context.getIndexBuffer().createBuffer(context.getDevice(), indexBufferSize,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true)) {

        Logger::logError("Error creating index buffer!");
        return false;
    }
    context.setIndexOffset(0);

    return true;
}

void VulkanShader::setModel(VulkanContext &context, const Mat4 &model) {
    unsigned int imageIndex = context.getImageIndex();
    VkCommandBuffer commandBuffer = context.getCurrentCommandBuffer().getHandle();
    vkCmdPushConstants(commandBuffer, pipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(model), &model);
}

bool VulkanShader::initialize(VulkanContext &context, ResourceSystem& resources) {
    const String stageTypeStrings[STAGE_COUNT] = {"vert", "frag"};
    constexpr VkShaderStageFlagBits stageTypes[STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (unsigned int i = 0; i < STAGE_COUNT; i++) {
        if (!createShaderModule(context, FF_MATERIAL_SHADER, stageTypeStrings[i], stageTypes[i], i, resources)) {
            Logger::logError("Unable to create " + stageTypeStrings[i] + " for " + FF_MATERIAL_SHADER);
            return false;
        }
    }

    materialStates.initialize(INITIAL_MATERIALS);

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

    samplerUses[0] = TEXTURE_USE_MAP_DIFFUSE;

    //Entity descriptors
    VkDescriptorType descriptorTypes[DESCRIPTOR_COUNT] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};
    VkDescriptorSetLayoutBinding bindings[DESCRIPTOR_COUNT];
    FF_Memory::ff_clear(&bindings, sizeof(VkDescriptorSetLayoutBinding) * DESCRIPTOR_COUNT);
    for (unsigned int i = 0; i < DESCRIPTOR_COUNT; i++) {
        bindings[i].binding = i;
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = descriptorTypes[i];
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = DESCRIPTOR_COUNT;
    layoutInfo.pBindings = bindings;
    VulkanUtils::vulkanCheck(vkCreateDescriptorSetLayout(context.getDevice().getLogicalDevice(), &layoutInfo, nullptr, &entityDescriptorLayout));

    VkDescriptorPoolSize entityPoolSizes[2];
    entityPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    entityPoolSizes[0].descriptorCount = INITIAL_MATERIALS;
    entityPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    entityPoolSizes[1].descriptorCount = INITIAL_MATERIALS * SAMPLER_COUNT;

    VkDescriptorPoolCreateInfo entityPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    entityPoolInfo.poolSizeCount = 2;
    entityPoolInfo.pPoolSizes = entityPoolSizes;
    entityPoolInfo.maxSets = INITIAL_MATERIALS;
    entityPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VulkanUtils::vulkanCheck(vkCreateDescriptorPool(context.getDevice().getLogicalDevice(), &entityPoolInfo, nullptr, &entityDescriptorPool));

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

    //attributes
    unsigned int offset = 0;
    constexpr int attributeCount = 2; //Attribute count is equal to the number of fields in Vertex3d.
    VkVertexInputAttributeDescription attributeDescriptions[attributeCount];
    constexpr VkFormat formats[attributeCount] = {VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT};
    constexpr unsigned long sizes[attributeCount] = {sizeof(Vector3f), sizeof(Vector2f)};
    for (unsigned int i = 0; i < attributeCount; i++) {
        attributeDescriptions[i].binding = 0;
        attributeDescriptions[i].location = i;
        attributeDescriptions[i].format = formats[i];
        attributeDescriptions[i].offset = offset;
        offset += sizes[i];
    }

    //Create descriptor set layouts
    constexpr int descriptorSetLayoutCount = 2;
    VkDescriptorSetLayout layouts[descriptorSetLayoutCount] = {globalDescriptorSetLayout, entityDescriptorLayout};

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

    unsigned int imageCount = context.getSwapchain().getImageCount();
    unsigned int deviceLocalBits = context.getDevice().supportsDeviceLocalBit() ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
    if (!globalUniformBuffer.createBuffer(context.getDevice(), sizeof(GlobalUniform) * imageCount,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | deviceLocalBits,
        true)) {

        Logger::logError("Failed to create global uniform buffer!");
        return false;
        }

    VkDescriptorSetLayout globalLayouts[imageCount];
    for (int i = 0; i < imageCount; i++) {
        globalLayouts[i] = globalDescriptorSetLayout;
    }
    VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocateInfo.descriptorPool = globalDescriptorPool;
    allocateInfo.descriptorSetCount = imageCount;
    allocateInfo.pSetLayouts = globalLayouts;
    globalDescriptorSets.initialize(imageCount);
    VulkanUtils::vulkanCheck(vkAllocateDescriptorSets(context.getDevice().getLogicalDevice(), &allocateInfo, globalDescriptorSets.getData()));

    //THE BUFFER IS HARD LIMITED TO 4096 FOR NOW! THE MATERIAL ARRAY IS DYNAMIC, SO ISSUES MAY OCCUR WAAAAAY LATER DOWN THE LINE!
    if (!entityUniformBuffer.createBuffer(context.getDevice(), sizeof(MaterialUniform) * 4096,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        true)) {

        Logger::logError("Entity buffer failed to create for shader.");
        return false;
    }

    return true;
}

void VulkanShader::destroy(VulkanContext& context) {
    vkDestroyDescriptorPool(context.getDevice().getLogicalDevice(), entityDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(context.getDevice().getLogicalDevice(), entityDescriptorLayout, nullptr);

    context.getVertexBuffer().destroyBuffer(context.getDevice());
    context.getIndexBuffer().destroyBuffer(context.getDevice());

    globalUniformBuffer.destroyBuffer(context.getDevice());
    entityUniformBuffer.destroyBuffer(context.getDevice());

    pipeline.destroyPipeline(context.getDevice());

    vkDestroyDescriptorPool(context.getDevice().getLogicalDevice(), globalDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(context.getDevice().getLogicalDevice(), globalDescriptorSetLayout, nullptr);

    for (auto & stage : stages) {
        vkDestroyShaderModule(context.getDevice().getLogicalDevice(), stage.handle, nullptr);
        stage.handle = nullptr;
    }
}

void VulkanShader::use(const VulkanContext& context) const {
    pipeline.bindPipeline(context.getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void VulkanShader::updateGlobalState(VulkanContext &context) {
    VkCommandBuffer commandBuffer = context.getCurrentCommandBuffer().getHandle();
    VkDescriptorSet globalDescriptorSet = globalDescriptorSets[context.getImageIndex()];

    constexpr unsigned int range = sizeof(GlobalUniform);
    const unsigned long offset = sizeof(GlobalUniform) * context.getImageIndex();

    globalUniformBuffer.loadBufferData(context.getDevice(), offset, range, &globalUBO);

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = globalUniformBuffer.getBuffer();
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