//
// Created by cmorg on 7/2/2026.
//

#include "VulkanBackend.h"

#include <cassert>
#include <cstring>
#include <iomanip>

#include "VulkanBackendShader.h"
#include "../../Library/Logger.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Memory/FF_Memory.h"

VulkanContext VulkanBackend::vulkanContext{};

constexpr unsigned int GLOBAL_DESCRIPTOR_SET_INDEX = 0;
constexpr unsigned int  INSTANCE_DESCRIPTOR_SET_INDEX = 1;

VkBool32 VulkanBackend::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData) {

    switch (messageSeverity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Logger::logError(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Logger::logWarn(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Logger::logInfo(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            Logger::logDebug(callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}

bool VulkanBackend::createSurface(const Platform& platform) {
    return platform.createSurface();
}

bool VulkanBackend::recreateSwapchain() {
    if (vulkanContext.getSwapchain().isRecreatingSwapchain()) {
        Logger::logDebug("Recreate swapchain was called while already recreating.");
        return false;
    }
    if (vulkanContext.getFrameBufferWidth() == 0 || vulkanContext.getFrameBufferHeight() == 0) {
        Logger::logDebug("Window is too small to recreate swapchain.");
        return false;
    }

    vulkanContext.getSwapchain().enableRecreateSwapchain();

    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    vulkanContext.clearImagesInFlight();

    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        if (vulkanContext.isCommandBufferValid(i) && vulkanContext.getCommandBuffer(i).getHandle()) {
            vulkanContext.getCommandBuffer(i).freeCommandBuffer(vulkanContext.getDevice());
        }
    }

    vulkanContext.getSwapchain().destroySwapchain(vulkanContext.getDevice());

    vulkanContext.getDevice().querySwapChainSupport(vulkanContext.getDevice().getPhysicalDevice(), vulkanContext.getSurface(), vulkanContext.getDevice().getSwapChainSupportInfo());
    vulkanContext.getSwapchain().detectDepthFormat(vulkanContext.getDevice());

    vulkanContext.getSwapchain().createSwapchain(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getDevice(), vulkanContext.getSurface(), vulkanContext.getCurrentFrame(), this);

    vulkanContext.getSwapchain().finishResize();

    if (vulkanContext.resizeRenderTargetsEvent.hasListeners()) {
        vulkanContext.resizeRenderTargetsEvent.call();
    }

    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        vulkanContext.getCommandBuffer(i).allocateCommandBuffer(true, vulkanContext.getDevice());
    }

    vulkanContext.getSwapchain().finishRecreateSwapchain();

    return true;
}

bool VulkanBackend::swapchainAcquireNextImageIndex(const unsigned long timeout, VkSemaphore semaphore, VkFence fence, unsigned int& outImageIndex) {
    const VkResult result = vkAcquireNextImageKHR(vulkanContext.getDevice().getLogicalDevice(), vulkanContext.getSwapchain().getSwapchain(), timeout, semaphore, fence, &outImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        Logger::logFatal("Failed to acquire swapchain image!");
        return false;
    }

    return true;
}

void VulkanBackend::presentSwapchain() {
    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vulkanContext.getCurrentQueueCompleteSemaphore();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkanContext.getSwapchain().getSwapchain();
    presentInfo.pImageIndices = &vulkanContext.getImageIndex();
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(vulkanContext.getDevice().getPresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        Logger::logFatal("Failed to present swapchain image!");
    }

    //loop the current frame
    vulkanContext.setCurrentFrame((vulkanContext.getCurrentFrame() + 1) % vulkanContext.getSwapchain().getMaxFramesInFlight());
}

void VulkanBackend::allocateCommandBuffers() {
    vulkanContext.createCommandBuffers();

    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        VulkanCommandBuffer& commandBuffer = vulkanContext.getCommandBuffer(i);
        if (commandBuffer.getHandle()) {
            commandBuffer.freeCommandBuffer(vulkanContext.getDevice());
        }
        commandBuffer.allocateCommandBuffer(true, vulkanContext.getDevice());
    }

    Logger::logInfo("Vulkan command buffers created and allocated.");
}

bool VulkanBackend::uploadRangeOfData(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer &buffer, unsigned long& outOffset, const unsigned long size, const void *data) {
    //Allocate buffer space
    if (!buffer.allocate(size, outOffset)) {
        Logger::logError("Failed to allocate data range for upload.");
        return false;
    }

    //Create staging buffer
    constexpr VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer stagingBuffer{};
    stagingBuffer.createBuffer(vulkanContext.getDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true);

    //Load staging buffer
    stagingBuffer.loadBufferData(vulkanContext.getDevice(), 0, size, data);
    stagingBuffer.copyBufferData(vulkanContext.getDevice(), pool, fence, queue, stagingBuffer.getBuffer(), 0, buffer.getBuffer(), outOffset, size);
    stagingBuffer.destroyBuffer(vulkanContext.getDevice());

    return true;
}

bool VulkanBackend::freeRangeOfData(VulkanBuffer &buffer, const unsigned long offset, const unsigned long size) {
    return buffer.free(size, offset);
}

bool VulkanBackend::beginRenderpass(Renderpass &renderpass, RenderTarget &target) {
    VulkanCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer();
    const auto vulkanRenderpass = reinterpret_cast<VulkanRenderpass *>(renderpass.getData());
    if (!vulkanRenderpass) {
        Logger::logFatal("Failed to cast Vulkan Renderpass in Renderpass: " + std::to_string(renderpass.getId()));
        return false;
    }

    VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = vulkanRenderpass->getHandle();
    beginInfo.framebuffer = static_cast<VkFramebuffer>(target.framebuffer);
    beginInfo.renderArea.offset.x = static_cast<int>(renderpass.getRenderArea().x);
    beginInfo.renderArea.offset.y = static_cast<int>(renderpass.getRenderArea().y);
    beginInfo.renderArea.extent.width = static_cast<int>(renderpass.getRenderArea().z);
    beginInfo.renderArea.extent.height = static_cast<int>(renderpass.getRenderArea().w);

    beginInfo.clearValueCount = 0;
    beginInfo.pClearValues = nullptr;

    VkClearValue clearValues[2]{};

    if (renderpass.hasFlag(RENDERPASS_CLEAR_COLOR)) {
        FF_Memory::ff_copy(clearValues[beginInfo.clearValueCount].color.float32, renderpass.getClearColor().elements, sizeof(float) * 4);
    }
    beginInfo.clearValueCount++; //Always increment regardless of result
    if (renderpass.hasFlag(RENDERPASS_CLEAR_DEPTH)) {
        FF_Memory::ff_copy(clearValues[beginInfo.clearValueCount].color.float32, renderpass.getClearColor().elements, sizeof(float) * 4);
        clearValues[beginInfo.clearValueCount].depthStencil.depth = vulkanRenderpass->getDepth();

        const bool doClearStencil = renderpass.hasFlag(RENDERPASS_CLEAR_STENCIL);
        clearValues[beginInfo.clearValueCount].depthStencil.stencil = doClearStencil ? vulkanRenderpass->getStencil() : 0;
        beginInfo.clearValueCount++;
    }

    beginInfo.pClearValues = beginInfo.clearValueCount > 0 ? clearValues : nullptr;

    vkCmdBeginRenderPass(commandBuffer.getHandle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.setState(IN_RENDER_PASS);

    return true;
}

bool VulkanBackend::endRenderpass(Renderpass &renderpass) {
    VulkanCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer();

    vkCmdEndRenderPass(commandBuffer.getHandle());
    commandBuffer.setState(RECORDING);

    return true;
}

bool VulkanBackend::createShader(Shader &shader, ShaderConfig& config, Renderpass &renderpass, const unsigned char stageCount, DynamicArray<String> &stageFileNames, DynamicArray<ShaderStage> &stages) {
    shader.setBackendShader(FF_Memory::ff_allocate_class<VulkanBackendShader>(sizeof(VulkanBackendShader), RENDER));

    VkShaderStageFlags vkStages[VULKAN_SHADER_MAX_STAGES]{};

    for (unsigned char i = 0; i < stageCount; i++) {
        switch (stages[i]) {
            case SHADER_STAGE_FRAGMENT: {
                vkStages[i] = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            }
            case SHADER_STAGE_VERTEX: {
                vkStages[i] = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            }
            case SHADER_STAGE_GEOMETRY: {
                Logger::logWarn("SHADER_STAGE_GEOMETRY is not supported at this time.");
                vkStages[i] = VK_SHADER_STAGE_GEOMETRY_BIT;
                break;
            }
            case SHADER_STAGE_COMPUTE: {
                Logger::logWarn("SHADER_STAGE_COMPUTE is not supported at this time.");
                vkStages[i] = VK_SHADER_STAGE_COMPUTE_BIT;
                break;
            }
        }
    }

    constexpr unsigned int maxDescriptorAllocationCount = 1024;

    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();
    const auto pass = reinterpret_cast<VulkanRenderpass *>(renderpass.getData());

    //Uniform counts
    for (unsigned int i = 0; i < config.uniforms.getLength(); i++) {
        const ShaderScope scope = config.uniforms[i].scope;
        if (config.uniforms[i].type == SHADER_UNIFORM_TYPE_SAMPLER) {
            backendShader->incrementSamplerCount(scope);
        } else {
            backendShader->incrementUniformCount(scope);
        }
    }

    backendShader->setRenderpass(*pass);
    backendShader->setMaxDescriptorCount(maxDescriptorAllocationCount);

    if (!backendShader->setStages(stageCount, stages, stageFileNames)) return false;

    backendShader->initializeDescriptorSets(vulkanContext.getSwapchain().getImageCount());

    backendShader->setPoolSizes();

    if (backendShader->getGlobalUniformCount() > 0 || backendShader->getGlobalSamplerCount() > 0) {
        VulkanDescriptorSetConfig& setConfig = backendShader->getDescriptorSetConfig(backendShader->getConfig().descriptorSetCount);

        if (backendShader->getGlobalUniformCount() > 0) {
            const unsigned char bindingIndex = setConfig.bindingCount;
            setConfig.bindings[bindingIndex].binding = bindingIndex;
            setConfig.bindings[bindingIndex].descriptorCount = 1;
            setConfig.bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            setConfig.bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            setConfig.bindingCount++;
        }

        if (backendShader->getGlobalSamplerCount() > 0) {
            const unsigned char bindingIndex = setConfig.bindingCount;
            setConfig.bindings[bindingIndex].binding = bindingIndex;
            setConfig.bindings[bindingIndex].descriptorCount = backendShader->getGlobalSamplerCount();
            setConfig.bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            setConfig.bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            setConfig.samplerBindingIndex = bindingIndex;
            setConfig.bindingCount++;
        }

        backendShader->getConfig().descriptorSetCount++;
    }

    if (backendShader->getInstanceUniformCount() > 0 || backendShader->getInstanceSamplerCount() > 0) {
        VulkanDescriptorSetConfig& setConfig = backendShader->getDescriptorSetConfig(backendShader->getConfig().descriptorSetCount);

        if (backendShader->getInstanceUniformCount() > 0) {
            const unsigned char bindingIndex = setConfig.bindingCount;
            setConfig.bindings[bindingIndex].binding = bindingIndex;
            setConfig.bindings[bindingIndex].descriptorCount = 1;
            setConfig.bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            setConfig.bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            setConfig.bindingCount++;
        }

        if (backendShader->getInstanceSamplerCount() > 0) {
            const unsigned char bindingIndex = setConfig.bindingCount;
            setConfig.bindings[bindingIndex].binding = bindingIndex;
            setConfig.bindings[bindingIndex].descriptorCount = backendShader->getInstanceSamplerCount();
            setConfig.bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            setConfig.bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            setConfig.samplerBindingIndex = bindingIndex;
            setConfig.bindingCount++;
        }

        backendShader->getConfig().descriptorSetCount++;
    }

    //Invalidate Instance States here if it ever becomes a problem.

    backendShader->getConfig().cullMode = config.cullMode;

    return true;
}

bool VulkanBackend::initializeShader(Shader &shader) {
    VkAllocationCallbacks* vkAllocator = nullptr;
    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();

    for (unsigned int i = 0; i < backendShader->getStageCount(); i++) {
        if (!createModule(backendShader->getConfig().stages[i], backendShader->getStage(i))) {
            Logger::logError("Failed to create " + backendShader->getConfig().stages[i].fileName + " for shader " + shader.getName());
            return false;
        }
    }

    static VkFormat* types = nullptr;
    static VkFormat t[11];
    if (!types) {
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32] = VK_FORMAT_R32_SFLOAT;
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32_2] = VK_FORMAT_R32G32_SFLOAT;
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32_3] = VK_FORMAT_R32G32B32_SFLOAT;
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32_4] = VK_FORMAT_R32G32B32A32_SFLOAT;
        t[SHADER_ATTRIBUTE_TYPE_INT8] = VK_FORMAT_R8_SINT;
        t[SHADER_ATTRIBUTE_TYPE_UINT8] = VK_FORMAT_R8_UINT;
        t[SHADER_ATTRIBUTE_TYPE_INT16] = VK_FORMAT_R16_SINT;
        t[SHADER_ATTRIBUTE_TYPE_UINT16] = VK_FORMAT_R16_UINT;
        t[SHADER_ATTRIBUTE_TYPE_INT32] = VK_FORMAT_R32_SINT;
        t[SHADER_ATTRIBUTE_TYPE_UINT32] = VK_FORMAT_R32_UINT;
        types = t;
    }

    //Process attributes
    const unsigned int attributeCount = shader.getAttributes().getLength();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < attributeCount; i++) {
        backendShader->setAttribute(i, types[shader.getAttribute(i).type], offset);
        offset += shader.getAttribute(i).size;
    }

    //Create descriptor pool
    if (!backendShader->createDescriptorPool(vulkanContext.getDevice(), vkAllocator)) return false;

    //Create descriptor set layouts
    for (unsigned int i = 0; i < backendShader->getConfig().descriptorSetCount; i++) {
        if (!backendShader->createDescriptorSetLayout(i, vulkanContext.getDevice(), vkAllocator)) return false;
    }

    //Viewport
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = static_cast<float>(vulkanContext.getFrameBufferHeight());
    viewport.width = static_cast<float>(vulkanContext.getFrameBufferWidth());
    viewport.height = static_cast<float>(vulkanContext.getFrameBufferHeight());
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    //Scissor
    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = vulkanContext.getFrameBufferWidth();
    scissor.extent.height = vulkanContext.getFrameBufferHeight();

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo[VULKAN_SHADER_MAX_STAGES]{};
    for (unsigned int i = 0; i < backendShader->getStageCount(); i++) {
        shaderStageCreateInfo[i] = backendShader->getStage(i).shaderStageCreateInfo;
    }

    const bool result = backendShader->createPipeline(shader.getAttributeStride(), shader.getAttributeCount(),
        shaderStageCreateInfo, viewport, scissor, shader.getPushConstantRangeCount(), shader.getPushConstantRanges(),
        vulkanContext.getDevice());

    if (!result) {
        Logger::logError("Failed to load pipeline for shader.");
        return false;
    }

    shader.setRequiredAlignment(vulkanContext.getDevice().getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment);
    shader.setGlobalStride();
    shader.setInstanceStride();

    const unsigned int deviceLocalBits = vulkanContext.getDevice().supportsDeviceLocalBit() ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
    const unsigned long totalBufferSize = shader.getGlobalStride() + (shader.getInstanceStride() * MAX_MATERIAL_COUNT);
    if (!backendShader->getUniformBuffer().createBuffer(vulkanContext.getDevice(), totalBufferSize,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | deviceLocalBits,
        true)) {

        Logger::logError("Failed to create uniform buffer for shader.");
        return false;
    }
    if (!backendShader->getUniformBuffer().allocate(shader.getGlobalStride(), shader.getGlobalOffset())) {
        Logger::logError("Failed to allocate space for a uniform buffer for a shader.");
        return false;
    }

    backendShader->finalizeBuffer(vulkanContext.getDevice());
    backendShader->finalizeDescriptorSets(vulkanContext.getSwapchain().getImageCount(), GLOBAL_DESCRIPTOR_SET_INDEX, vulkanContext.getDevice());

    return true;
}

void VulkanBackend::destroyShader(Shader &shader) {
    if (!shader.getBackendShader()) return;

    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();

    backendShader->shutdown(vulkanContext.getDevice(), nullptr);

    FF_Memory::ff_free_class<VulkanBackendShader>(shader.getBackendShader(), sizeof(VulkanBackendShader), RENDER);
    shader.setBackendShader(nullptr);
}

bool VulkanBackend::useShader(Shader &shader) {
    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();
    backendShader->getPipeline().bindPipeline(vulkanContext.getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS);
    return true;
}

bool VulkanBackend::bindShaderGlobals(Shader &shader) {
    shader.setBoundOffset(shader.getGlobalOffset());
    return true;
}

void VulkanBackend::bindShaderInstance(Shader &shader, unsigned instanceId) {
    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();

    shader.setBoundInstanceId(instanceId);
    const VulkanShaderInstanceState& state = backendShader->getInstanceState(instanceId);
    shader.setBoundOffset(state.offset);
}

bool VulkanBackend::setUniform(Shader &shader, ShaderUniform &uniform, void *value) {
    const auto backendShader = shader.getBackendShader<VulkanBackendShader>();

    if (uniform.type == SHADER_UNIFORM_TYPE_SAMPLER) {
        if (uniform.scope == SHADER_SCOPE_GLOBAL) {
            shader.setTextureMap(uniform.location, static_cast<TextureMap*>(value));
        } else {
            backendShader->getInstanceState(shader.getBoundInstanceId()).instanceTextureMaps[uniform.location] = static_cast<TextureMap*>(value);
        }
    } else {
        if (uniform.scope == SHADER_SCOPE_LOCAL) {
            VkCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer().getHandle();
            vkCmdPushConstants(commandBuffer, backendShader->getPipeline().getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, uniform.offset, uniform.size, value);
        } else {
            auto* address = static_cast<unsigned char*>(backendShader->getUniformBufferMemoryBlock());
            address += shader.getBoundOffset() + uniform.offset;
            FF_Memory::ff_copy(address, value, uniform.size);
        }
    }

    return true;
}

bool VulkanBackend::applyShaderGlobals(Shader &shader) {
    const unsigned int imageIndex = vulkanContext.getImageIndex();
    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();
    VkCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer().getHandle();
    VkDescriptorSet& globalDescriptor = backendShader->getDescriptorSet(imageIndex);

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = backendShader->getUniformBuffer().getBuffer();
    bufferInfo.offset = shader.getGlobalOffset();
    bufferInfo.range = shader.getGlobalStride();

    VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstSet = globalDescriptor;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    VkWriteDescriptorSet descriptorWrites[2]{};
    descriptorWrites[0] = descriptorWrite;

    unsigned int globalSetBindingCount = backendShader->getConfig().descriptorSets[GLOBAL_DESCRIPTOR_SET_INDEX].bindingCount;
    if (globalSetBindingCount > 1) {
        globalSetBindingCount = 1;
        Logger::logError("Global image samplers are not supported at this time.");
    }

    vkUpdateDescriptorSets(vulkanContext.getDevice().getLogicalDevice(), globalSetBindingCount, descriptorWrites, 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendShader->getPipeline().getPipelineLayout(), 0, 1, &globalDescriptor, 0, nullptr);
    return true;
}

bool VulkanBackend::applyShaderInstance(Shader &shader, const bool update) {
    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();

    if (backendShader->getInstanceUniformCount() == 0 && backendShader->getInstanceSamplerCount() == 0) {
        Logger::logError("Cannot apply shader because the shader does not support instances.");
        return false;
    }

    const unsigned int imageIndex = vulkanContext.getImageIndex();
    VkCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer().getHandle();

    VulkanShaderInstanceState& state = backendShader->getInstanceState(shader.getBoundInstanceId());
    VkDescriptorSet& instanceDescriptor = state.descriptorSetState.descriptorSets[imageIndex];

    if (update) {
        VkWriteDescriptorSet descriptorWrites[2]{};
        unsigned int descriptorCount = 0;
        unsigned int descriptorIndex = 0;

        if (backendShader->getInstanceUniformCount() > 0) {
            unsigned char &instanceGeneration = state.descriptorSetState.descriptorStates[descriptorIndex].generations[imageIndex];

            VkWriteDescriptorSet instanceDescriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            VkDescriptorBufferInfo bufferInfo{};
            if (instanceGeneration == INVALID_ID_U8) {
                bufferInfo.buffer = backendShader->getUniformBuffer().getBuffer();
                bufferInfo.offset = state.offset;
                bufferInfo.range = shader.getInstanceStride();

                instanceDescriptorWrite.dstSet = instanceDescriptor;
                instanceDescriptorWrite.dstBinding = descriptorIndex;
                instanceDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                instanceDescriptorWrite.descriptorCount = 1;
                instanceDescriptorWrite.pBufferInfo = &bufferInfo;

                descriptorWrites[descriptorCount] = instanceDescriptorWrite;
                descriptorCount++;

                instanceGeneration = 1;
            }

            descriptorIndex++;
        }

        VkDescriptorImageInfo imageInfos[VULKAN_SHADER_MAX_GLOBAL_TEXTURES]{};
        VkWriteDescriptorSet samplerDescriptor{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        if (backendShader->getInstanceSamplerCount() > 0) {
            const unsigned char samplerBindingIndex = backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].samplerBindingIndex;
            const unsigned int totalSamplerCount = backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].bindings[samplerBindingIndex].descriptorCount;
            unsigned int updateSamplerCount = 0;
            for (unsigned int i = 0; i < totalSamplerCount; i++) {
                const TextureMap* map = backendShader->getInstanceState(shader.getBoundInstanceId()).instanceTextureMaps[i];
                const Texture* texture = map->texture;
                if (!texture) {
                    Logger::logFatal("Cannot apply shader instance because texture is null!");
                    return false;
                }
                VulkanImage& image = *static_cast<VulkanImage *>(texture->data);
                imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[i].imageView = image.getImageView();
                imageInfos[i].sampler = static_cast<VkSampler>(map->data);

                updateSamplerCount++;
            }

            samplerDescriptor.dstSet = instanceDescriptor;
            samplerDescriptor.dstBinding = descriptorIndex;
            samplerDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerDescriptor.descriptorCount = updateSamplerCount;
            samplerDescriptor.pImageInfo = imageInfos;
            descriptorWrites[descriptorCount] = samplerDescriptor;
            descriptorCount++;
        }

        if (descriptorCount > 0) {
            vkUpdateDescriptorSets(vulkanContext.getDevice().getLogicalDevice(), descriptorCount, descriptorWrites, 0,nullptr);
        }
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendShader->getPipeline().getPipelineLayout(), 1, 1, &instanceDescriptor, 0, nullptr);
    return true;
}

Renderpass * VulkanBackend::getRenderpass(const String name) {
    if (name.empty()) {
        Logger::logError("Get Renderpass requires a name!");
        return nullptr;
    }

    Renderpass* renderpass = vulkanContext.getRenderpass(name);
    if (!renderpass) {
        Logger::logWarn("No renderpass by the name: " + name + " could be found!");
        return nullptr;
    }

    return renderpass;
}

Texture * VulkanBackend::getWindowAttachment(unsigned char index) {
    if (index >= vulkanContext.getSwapchain().getImageCount()) {
        Logger::logFatal("Cannot obtain attachment index that is out of range. Got " + std::to_string(index) + " but the size is " + std::to_string(vulkanContext.getSwapchain().getImageCount()));
        return nullptr;
    }

    return vulkanContext.getSwapchain().getTexture(index);
}

Texture * VulkanBackend::getDepthAttachment() {
    return vulkanContext.getSwapchain().getDepthTexture();
}

unsigned char VulkanBackend::getWindowAttachmentIndex() {
    return static_cast<unsigned char>(vulkanContext.getImageIndex());
}

void VulkanBackend::createRenderTarget(const unsigned char attachmentCount, DynamicArray<Texture *>& attachments, Renderpass &renderpass, const unsigned width, const unsigned height, RenderTarget &outTarget) {
    const auto pass = reinterpret_cast<VulkanRenderpass *>(renderpass.getData());
    if (!pass) {
        Logger::logFatal("Failed to cast to Vulkan Render Pass while creating a render target for renderpass: " + std::to_string(renderpass.getId()));
        return;
    }

    VkImageView attachmentViews[32]{};

    outTarget.attachmentCount = attachmentCount;
    if (outTarget.attachments.getCapacity() == 0) {
        outTarget.attachments.initialize(attachmentCount);

        for (unsigned int i = 0; i < attachmentCount; i++) {
            attachmentViews[i] = static_cast<VulkanImage *>(attachments[i]->data)->getImageView();
            outTarget.attachments.push(attachments[i]);
        }
    } else {
        for (unsigned int i = 0; i < attachmentCount; i++) {
            attachmentViews[i] = static_cast<VulkanImage *>(attachments[i]->data)->getImageView();
            outTarget.attachments[i] = attachments[i];
        }
    }

    VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferCreateInfo.renderPass = pass->getHandle();
    framebufferCreateInfo.attachmentCount = attachmentCount;
    framebufferCreateInfo.pAttachments = attachmentViews;
    framebufferCreateInfo.width = width;
    framebufferCreateInfo.height = height;
    framebufferCreateInfo.layers = 1;

    VulkanUtils::vulkanCheck(vkCreateFramebuffer(vulkanContext.getDevice().getLogicalDevice(), &framebufferCreateInfo, nullptr, reinterpret_cast<VkFramebuffer *>(&outTarget.framebuffer)));
}

void VulkanBackend::destroyRenderTarget(RenderTarget &target, const bool freeMemory) {
    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    if (target.framebuffer) {
        vkDestroyFramebuffer(vulkanContext.getDevice().getLogicalDevice(), static_cast<VkFramebuffer>(target.framebuffer), nullptr);
        target.framebuffer = nullptr;
    }

    if (freeMemory) {
        //for (Texture* texture : target.attachments) {
        //    if (texture->data) {
        //        FF_Memory::ff_free(texture, sizeof(Texture), TEXTURE);
        //   }
        //}
        target.attachments.shutdown();
        target.attachmentCount = 0;
    }
}

void VulkanBackend::createRenderpass(Renderpass &outRenderpass, float depth, unsigned stencil, bool hasPreviousPass, bool hasNextPass) {
    auto pass = FF_Memory::ff_allocate_class<VulkanRenderpass>(sizeof(VulkanRenderpass), RENDER);
    pass->setupFramebuffers(vulkanContext.getSwapchain().getImageCount());

    outRenderpass.setData(pass);
    pass->setPreviousPass(hasPreviousPass);
    pass->setNextPass(hasNextPass);
    pass->setDepth(depth);
    pass->setStencil(stencil);

    //Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    //Attachments
    unsigned int attachmentCount = 0;
    VkAttachmentDescription attachmentDescriptions[2]{};

    //Color attachment
    bool doClearColor = outRenderpass.hasFlag(RENDERPASS_CLEAR_COLOR);
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vulkanContext.getSwapchain().getImageFormat().format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = doClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = hasPreviousPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = hasNextPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.flags = 0;

    attachmentDescriptions[attachmentCount] = colorAttachment;
    attachmentCount++;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    //Depth attachment
    VkAttachmentReference depthAttachmentReference{};
    bool doClearDepth = outRenderpass.hasFlag(RENDERPASS_CLEAR_DEPTH);
    if (doClearDepth) {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = vulkanContext.getDevice().getDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        if (hasPreviousPass) {
            depthAttachment.loadOp = doClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        } else {
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachmentDescriptions[attachmentCount] = depthAttachment;
        attachmentCount++;

        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.pDepthStencilAttachment = &depthAttachmentReference;
    } else {
        //Cannot zero out [1] here because Vulkan will complain.
        subpass.pDepthStencilAttachment = nullptr;
    }

    //Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;

    //Multisampling
    subpass.pResolveAttachments = nullptr;

    //Attachements not used in this subpass but are needed for the next subpass
    subpass.preserveAttachmentCount = 0;
    subpass.pResolveAttachments = nullptr;

    //Dependencies for render pass
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    //Create render pass
    VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = attachmentCount;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;

    VulkanUtils::vulkanCheck(vkCreateRenderPass(vulkanContext.getDevice().getLogicalDevice(), &renderPassCreateInfo, nullptr, &pass->getHandle()));
}

void VulkanBackend::destroyRenderpass(Renderpass &renderpass) {
    if (!renderpass.getData()) return;

    VulkanRenderpass& pass = *reinterpret_cast<VulkanRenderpass *>(renderpass.getData());

    if (pass.getHandle() != VK_NULL_HANDLE) {
        pass.destroyFramebuffers(vulkanContext.getDevice());
        vkDestroyRenderPass(vulkanContext.getDevice().getLogicalDevice(), pass.getHandle(), nullptr);
        pass.shutdown();

        FF_Memory::ff_free_class<VulkanRenderpass>(&pass, sizeof(VulkanRenderpass), RENDER);
        renderpass.setData(nullptr);
    }
}

void VulkanBackend::resize(const unsigned short width, const unsigned short height) {
    vulkanContext.setWidth(width);
    vulkanContext.setHeight(height);
    vulkanContext.getSwapchain().resize();

    Logger::logInfo("Vulkan backend resized to  " + std::to_string(width) + "x" + std::to_string(height));
}

VulkanBackend::~VulkanBackend() {
    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    //Destroy buffers
    vulkanContext.getVertexBuffer().destroyBuffer(vulkanContext.getDevice());
    vulkanContext.getIndexBuffer().destroyBuffer(vulkanContext.getDevice());

    Logger::logDebug("Destroying sync objects");
    //Destroy sync objects
    vulkanContext.destroySyncObjects();

    Logger::logDebug("Destroying command buffers.");
    //Destroy command buffers
    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        VulkanCommandBuffer& commandBuffer = vulkanContext.getCommandBuffer(i);
        if (commandBuffer.getHandle()) {
            commandBuffer.freeCommandBuffer(vulkanContext.getDevice());
            commandBuffer.destroyHandle();
        }
    }
    vulkanContext.destroyCommandBuffers();

    //Destroy Render Targets
    //for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
    //    destroyRenderTarget(vulkanContext.getRenderTarget(i), true);
    //    destroyRenderTarget(vulkanContext.getSwapchain().getRenderTarget(i), true);
    //}

    //Destroy Renderpasses
    Logger::logDebug("Destroying Renderpasses and Framebuffers.");
    for (unsigned int i = 0; i < VULKAN_MAX_RENDERPASSES; i++) {
        Renderpass* renderpass = vulkanContext.getRenderpass(i);
        if (renderpass) destroyRenderpass(*renderpass);
    }

    //Destroy swap chain
    Logger::logDebug("Destroying Swapchain.");
    vulkanContext.getSwapchain().destroySwapchain(vulkanContext.getDevice());
    vulkanContext.destroyContext();
}

bool VulkanBackend::initialize(Platform &platform, const RendererBackendConfig& config, unsigned char& outRenderTargetCount, ResourceSystem* resources) {
    resourceSystemRef = resources;
    vulkanContext.initializeEvents();
    vulkanContext.initializeGeometry();

    //Connect refresh function here
    vulkanContext.resizeRenderTargetsEvent.subscribe(config.func);

    vulkanContext.setWidth(800);
    vulkanContext.setHeight(600);

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = config.appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
    appInfo.pEngineName = "FoxFire Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

    //Get required extensions
    DynamicArray<const char*> requiredExtensions{1};
    requiredExtensions.push(VK_KHR_SURFACE_EXTENSION_NAME);
    platform.getRequiredExtensions(requiredExtensions);
#if ENABLE_DEBUG_LOGGING == true
    requiredExtensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    Logger::logDebug("Required Extensions: ");
    for (const String& extension : requiredExtensions) {
        Logger::logDebug(extension);
    }
#endif

    DynamicArray<const char*> validationLayers{1};
    unsigned int layerCount = 0;

#if ENABLE_DEBUG_LOGGING == true
    Logger::logDebug("Debug mode enable. Starting validation layers.");

    validationLayers.push("VK_LAYER_KHRONOS_validation");
    //validationLayers.push("VK_LAYER_LUNARG_api_dump"); //Every vulkan call will be appended to the log
    layerCount = validationLayers.getLength();
    unsigned int availableLayerCount = 0;
    VulkanUtils::vulkanCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
    VkLayerProperties availableLayers[availableLayerCount];
    VulkanUtils::vulkanCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

    for (const char* requiredLayer : validationLayers) {
        Logger::logInfo("Searching for " + String(requiredLayer));
        bool found = false;
        for (const VkLayerProperties properties : availableLayers) {
            if (strcmp(requiredLayer, properties.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            Logger::logFatal("Validation layer " + String(requiredLayer) + " could not be found.");
            return false;
        }
    }

    Logger::logDebug("All required validation layers were found!");

#endif

    createInfo.enabledExtensionCount = requiredExtensions.getLength();
    createInfo.ppEnabledExtensionNames = requiredExtensions.getData();
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = validationLayers.getData();

    VulkanUtils::vulkanCheck(vkCreateInstance(&createInfo, nullptr, &vulkanContext.getInstance()));
    Logger::logInfo("Vulkan Instance Created Successfully.");

#if ENABLE_DEBUG_LOGGING == true
    Logger::logDebug("Creating Vulkan debugger.");
    constexpr unsigned int logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

    const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanContext.getInstance(), "vkCreateDebugUtilsMessengerEXT"));
    assert(function);
    VulkanUtils::vulkanCheck(function(vulkanContext.getInstance(), &debugCreateInfo, nullptr, &vulkanContext.getDebugMessenger()));
    Logger::logDebug("Vulkan debugger created successfully.");
#endif

    //Create surface
    Logger::logInfo("Creating Vulkan surface.");
    if (!createSurface(platform)) {
        Logger::logFatal("Failed to create surface for Vulkan.");
        return false;
    }
    Logger::logInfo("Created Vulkan surface successfully.");

    //Create device
    if (!vulkanContext.getDevice().createDevice(vulkanContext.getInstance(), vulkanContext.getSurface())) {
        Logger::logFatal("Failed to create Vulkan device.");
        return false;
    }

    vulkanContext.getSwapchain().createSwapchain(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getDevice(), vulkanContext.getSurface(), vulkanContext.getCurrentFrame(), this);

    outRenderTargetCount = vulkanContext.getSwapchain().getImageCount();

    vulkanContext.initializeRenderpasses();

    for (unsigned int i = 0; i < config.renderpassCount; i++) {
        Renderpass* renderpass = vulkanContext.addRenderpass(config.configs[i]);
        if (!renderpass) continue;

        createRenderpass(*renderpass, 1.0f, 0, !config.configs[i].prevName.empty(), !config.configs[i].nextName.empty());
    }

    Logger::logInfo("Creating and allocating command buffers");
    allocateCommandBuffers();

    //Sync objects
    Logger::logInfo("Creating fences");
    vulkanContext.createSyncObjects();

    for (unsigned char i = 0; i < vulkanContext.getSwapchain().getMaxFramesInFlight(); i++) {
        VkSemaphoreCreateInfo semCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(vulkanContext.getDevice().getLogicalDevice(), &semCreateInfo, nullptr, &vulkanContext.getImageAvailableSemaphores()[i]);

        VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VulkanUtils::vulkanCheck(vkCreateFence(vulkanContext.getDevice().getLogicalDevice(), &fenceCreateInfo, nullptr, &vulkanContext.getFenceInFlight(i)));
    }
    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        VkSemaphoreCreateInfo semCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(vulkanContext.getDevice().getLogicalDevice(), &semCreateInfo, nullptr, &vulkanContext.getQueueCompleteSemaphores()[i]);
    }

    //Set initial state to 0. This is allocated in createSyncObject().
    vulkanContext.clearImagesInFlight();

    createBuffers();

    Logger::logInfo("Vulkan renderer initialized");
    return true;
}

void VulkanBackend::setVersion(const GameInstance& gameInstance) {
    majorVersion = gameInstance.config.gameVersionMajor;
    minorVersion = gameInstance.config.gameVersionMinor;
    patchVersion = gameInstance.config.gameVersionPatch;
}

bool VulkanBackend::beginFrame(const float deltaTime) {
    vulkanContext.setDeltaTime(deltaTime);

    if (vulkanContext.getSwapchain().isRecreatingSwapchain()) {
        VkResult result = vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());
        if (!VulkanUtils::vulkanCheck(result)) {
            Logger::logError("Vulkan begin frame failed. vkDeviceWaitIdle #1 failed: " + VulkanUtils::getResultAsString(result, true));
            return false;
        }
        Logger::logInfo("Recreating swapchain.");
        return false;
    }

    if (vulkanContext.getSwapchain().needsResize()) {
        VkResult result = vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());
        if (!VulkanUtils::vulkanCheck(result)) {
            Logger::logError("Vulkan begin frame failed. vkDeviceWaitIdle #2 failed: " + VulkanUtils::getResultAsString(result, true));
            return false;
        }

        if (!recreateSwapchain()) {
            return false;
        }

        Logger::logInfo("Swapchain resized. Waiting for next frame.");
        return false;
    }

    VkResult result = vkWaitForFences(vulkanContext.getDevice().getLogicalDevice(), 1, &vulkanContext.getCurrentInFlightFence(), true, UINT64_MAX);
    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("In Flight Fence failed to wait! Error: " + VulkanUtils::getResultAsString(result, true));
    }

    if (!swapchainAcquireNextImageIndex(UINT32_MAX, vulkanContext.getCurrentImageAvailable(), nullptr, vulkanContext.getImageIndex())) {
        return false;
    }

    vulkanContext.getCurrentCommandBuffer().resetCommandBuffer();
    vulkanContext.getCurrentCommandBuffer().beginCommandBuffer(false, false, false);

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = static_cast<float>(vulkanContext.getFrameBufferHeight());
    viewport.width = static_cast<float>(vulkanContext.getFrameBufferWidth());
    viewport.height = -static_cast<float>(vulkanContext.getFrameBufferHeight());
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = vulkanContext.getFrameBufferWidth();
    scissor.extent.height = vulkanContext.getFrameBufferHeight();

    vkCmdSetViewport(vulkanContext.getCurrentCommandBuffer().getHandle(), 0, 1, &viewport);
    vkCmdSetScissor(vulkanContext.getCurrentCommandBuffer().getHandle(), 0, 1, &scissor);

    return true;
}

bool VulkanBackend::endFrame(const float deltaTime) {
    vulkanContext.getCurrentCommandBuffer().endCommandBuffer();

    //make sure the previous fence cannot grab this new frame
    if (vulkanContext.getCurrentImageInFlight() != nullptr) {
        VkResult result = vkWaitForFences(vulkanContext.getDevice().getLogicalDevice(), 1, vulkanContext.getCurrentImageInFlight(), true, UINT64_MAX);
        if (!VulkanUtils::vulkanCheck(result)) {
            Logger::logFatal("Image in flight failed to wait: " + VulkanUtils::getResultAsString(result, true));
            return false;
        }
    }

    vulkanContext.updateCurrentImageInFlight();
    VulkanUtils::vulkanCheck(vkResetFences(vulkanContext.getDevice().getLogicalDevice(), 1, &vulkanContext.getCurrentInFlightFence()));

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanContext.getCurrentCommandBuffer().getHandle();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vulkanContext.getCurrentQueueCompleteSemaphore();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vulkanContext.getCurrentImageAvailable();
    constexpr VkPipelineStageFlags flags[1]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(vulkanContext.getDevice().getGraphicsQueue(), 1, &submitInfo, vulkanContext.getCurrentInFlightFence());

    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("vkQueueSubmit failed with result: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    vulkanContext.getCurrentCommandBuffer().updateSubmittedCommandBuffer();
    presentSwapchain();

    return true;
}

void VulkanBackend::drawGeometry(const GeometryRenderData &data, Texture& defaultTexture, Material& defaultMaterial) {
    //Geometry must be valid
    if (!data.geometry || data.geometry->internalId == INVALID_ID_U32) {
        return;
    }

    const GeometryData& bufferData = vulkanContext.getGeometry(data.geometry->internalId);
    VulkanCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer();

    VkDeviceSize offsets[1] = {bufferData.vertexBufferOffset};
    vkCmdBindVertexBuffers(commandBuffer.getHandle(), 0, 1, &vulkanContext.getVertexBuffer().getBuffer(), offsets);

    if (bufferData.indexCount > 0) {
        vkCmdBindIndexBuffer(commandBuffer.getHandle(), vulkanContext.getIndexBuffer().getBuffer(), bufferData.indexBufferOffset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer.getHandle(), bufferData.indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer.getHandle(), bufferData.vertexCount, 1, 0, 0);
    }
}

void VulkanBackend::createTexture(const unsigned char *pixels, Texture &texture) {
    texture.generation = INVALID_ID_U32;

    texture.data = FF_Memory::ff_allocate_class<VulkanImage>(sizeof(VulkanImage), TEXTURE);
    VulkanImage& data = *static_cast<VulkanImage *>(texture.data);

    const unsigned int imageSize = texture.width * texture.height * texture.channelCount;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    data.createImage(texture.type, texture.width, texture.height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT, vulkanContext.getDevice());

    writeTextureData(texture, 0, imageSize, pixels);

    texture.generation++;
}

void VulkanBackend::destroyTexture(Texture &texture) {
    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    if (texture.data) {
        VulkanImage& data = *static_cast<VulkanImage *>(texture.data);

        data.destroy(vulkanContext.getDevice());

        FF_Memory::ff_free_class<VulkanImage>(texture.data, sizeof(VulkanImage), TEXTURE);
    }
}

bool VulkanBackend::createGeometry(Geometry &geometry, const unsigned int vertexSize, const unsigned int vertexCount, Vertex* vertices, const unsigned int indexSize, const unsigned int indexCount, void *indices) {
    if (vertexCount == 0) {
        Logger::logError("No Vertex data was supplied for geometry creation! Vertex Count: " + std::to_string(vertexCount));
        return false;
    }

    const bool isReupload = geometry.internalId != INVALID_ID_U32;
    GeometryData oldData{};
    GeometryData* data = nullptr;

    if (isReupload) {
        data = &vulkanContext.getGeometry(geometry.internalId);
        oldData.indexBufferOffset = data->indexBufferOffset;
        oldData.indexCount = data->indexCount;
        oldData.indexElementSize = data->indexElementSize;
        oldData.vertexBufferOffset = data->vertexBufferOffset;
        oldData.vertexCount = data->vertexCount;
        oldData.vertexElementSize = data->vertexElementSize;
    } else {
        const unsigned int index = vulkanContext.assignGeometry();
        geometry.internalId = index;
        vulkanContext.getGeometry(index).id = index;
        data = &vulkanContext.getGeometry(index);
    }

    if (!data) {
        Logger::logFatal("Vulkan failed to create geometry!");
        return false;
    }

    VkCommandPool pool = vulkanContext.getDevice().getCommandPool();
    VkQueue queue = vulkanContext.getDevice().getGraphicsQueue();

    //upload vertexes
    data->vertexCount = vertexCount;
    data->vertexElementSize = vertexSize;
    const unsigned int vertexTotalSize = data->vertexElementSize * vertexCount;
    if (!uploadRangeOfData(pool, nullptr, queue, vulkanContext.getVertexBuffer(), data->vertexBufferOffset, vertexTotalSize, vertices)) {
        Logger::logError("Failed to upload geometry vertex data!");
        return false;
    }

    //Upload indexes if they exist
    if (indexCount > 0 && indices) {
        data->indexCount = indexCount;
        data->indexElementSize = indexSize;
        const unsigned int indexTotalSize = data->indexElementSize * indexCount;
        if (!uploadRangeOfData(pool, nullptr, queue, vulkanContext.getIndexBuffer(), data->indexBufferOffset, indexTotalSize, indices)) {
            Logger::logError("Failed to upload geometry index data!");
            return false;
        }
    }

    if (data->generation == INVALID_ID_U32) {
        data->generation = 0;
    } else {
        data->generation++;
    }

    //Free old data
    if (isReupload) {
        freeRangeOfData(vulkanContext.getVertexBuffer(), oldData.vertexBufferOffset, oldData.vertexElementSize * oldData.vertexCount);
        if (oldData.indexCount > 0) {
            freeRangeOfData(vulkanContext.getIndexBuffer(), oldData.indexBufferOffset, oldData.indexElementSize * oldData.indexCount);
        }
    }

    return true;
}

void VulkanBackend::destroyGeometry(Geometry &geometry) {
    if (geometry.internalId == INVALID_ID_U32) return;

    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    GeometryData& data = vulkanContext.getGeometry(geometry.internalId);

    //Free vertexes
    freeRangeOfData(vulkanContext.getVertexBuffer(), data.vertexBufferOffset, data.vertexElementSize * data.vertexCount);

    //Free indices if they exist
    if (data.indexCount > 0) {
        freeRangeOfData(vulkanContext.getIndexBuffer(), data.indexBufferOffset, data.indexElementSize * data.indexCount);
    }

    //Reset data
    data = GeometryData{};
}

bool VulkanBackend::createBuffers() {
    VkMemoryPropertyFlags memoryPropertyFlags{VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    constexpr unsigned long vertexBufferSize = sizeof(Vertex3d) * 1024 * 1024; //Vertex Buffer should be 64mb with this
    if (!vulkanContext.getVertexBuffer().createBuffer(vulkanContext.getDevice(), vertexBufferSize,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true)) {

        Logger::logError("Error creating vertex buffer!");
        return false;
        }

    constexpr unsigned long indexBufferSize = sizeof(unsigned int) * 1024 * 1024;
    if (!vulkanContext.getIndexBuffer().createBuffer(vulkanContext.getDevice(), indexBufferSize,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true)) {

        Logger::logError("Error creating index buffer!");
        return false;
        }

    return true;
}

bool VulkanBackend::createModule(const VulkanShaderStageConfig &config, VulkanShaderStage &stage) const {
    Resource binaryResource{};

    if (!resourceSystemRef->load(config.fileName, RESOURCE_TYPE_BINARY, binaryResource)) {
        Logger::logError("Failed to read shader file " + config.fileName);
        return false;
    }

    stage.createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    stage.createInfo.codeSize = binaryResource.dataSize;
    stage.createInfo.pCode = static_cast<unsigned int *>(binaryResource.data);

    VulkanUtils::vulkanCheck(vkCreateShaderModule(vulkanContext.getDevice().getLogicalDevice(), &stage.createInfo, nullptr, &stage.handle));

    resourceSystemRef->unload(binaryResource);

    stage.shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.shaderStageCreateInfo.stage = config.stage;
    stage.shaderStageCreateInfo.module = stage.handle;
    stage.shaderStageCreateInfo.pName = "main";

    return true;
}

bool VulkanBackend::acquireTextureMapResources(TextureMap &textureMap) {
    VkSamplerCreateInfo info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    //Configurable
    info.minFilter = convertTextureFilterToVulkan("min", textureMap.filterMin);
    info.magFilter = convertTextureFilterToVulkan("mag", textureMap.filterMag);

    info.addressModeU = convertTextureRepeatToVulkan("U", textureMap.repeatU);
    info.addressModeV = convertTextureRepeatToVulkan("V", textureMap.repeatV);
    info.addressModeW = convertTextureRepeatToVulkan("W", textureMap.repeatW);

    //Not configurable
    info.anisotropyEnable = VK_TRUE;
    info.maxAnisotropy = 16;
    info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    info.unnormalizedCoordinates = VK_FALSE;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.mipLodBias = 0.0f;
    info.minLod = 0.0f;
    info.maxLod = 0.0f;

    VkResult result = vkCreateSampler(vulkanContext.getDevice().getLogicalDevice(), &info, nullptr, reinterpret_cast<VkSampler *>(&textureMap.data));
    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("An error occured while creating VkSampler: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    return true;
}

void VulkanBackend::releaseTextureMapResources(TextureMap &textureMap) {
    vkDestroySampler(vulkanContext.getDevice().getLogicalDevice(), static_cast<VkSampler>(textureMap.data), nullptr);
    textureMap.data = nullptr;
}

void VulkanBackend::createWritableTexture(Texture &texture) {
    texture.data = FF_Memory::ff_allocate_class<VulkanImage>(sizeof(VulkanImage), TEXTURE);
    VulkanImage& image = *static_cast<VulkanImage *>(texture.data);

    VkFormat imageFormat = convertChannelCountToFormat(texture.channelCount, VK_FORMAT_R8G8B8A8_UNORM);
    image.createImage(texture.type, texture.width, texture.height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT, vulkanContext.getDevice());

    texture.generation++;
}

void VulkanBackend::resizeTexture(Texture &texture, unsigned int width, unsigned int height) {
    if (!texture.data) return;

    VulkanImage& image = *static_cast<VulkanImage *>(texture.data);
    image.destroy(vulkanContext.getDevice());

    VkFormat imageFormat = convertChannelCountToFormat(texture.channelCount, VK_FORMAT_R8G8B8A8_UNORM);

    image.createImage(texture.type, width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT, vulkanContext.getDevice());

    texture.generation++;
}

void VulkanBackend::writeTextureData(Texture &texture, unsigned int offset, unsigned int size, const unsigned char *pixels) {
    const VulkanImage& image = *static_cast<VulkanImage *>(texture.data);
    VkDeviceSize imageSize = texture.width * texture.height * texture.channelCount * (texture.type == TEXTURE_CUBE ? 6 : 1);
    VkFormat imageFormat = convertChannelCountToFormat(texture.channelCount, VK_FORMAT_R8G8B8A8_UNORM);

    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer stagingBuffer{};
    stagingBuffer.createBuffer(vulkanContext.getDevice(), imageSize, usage, memoryFlags, true);
    stagingBuffer.loadBufferData(vulkanContext.getDevice(), 0, imageSize, pixels);

    VulkanCommandBuffer tempBuffer = VulkanCommandBuffer::allocateAndBeginSingleUseCommandBuffer(vulkanContext.getDevice());
    VkCommandPool pool = vulkanContext.getDevice().getCommandPool();
    VkQueue queue = vulkanContext.getDevice().getGraphicsQueue();

    image.transitionImageLayout(tempBuffer, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.type, vulkanContext.getDevice());
    image.copyFromBuffer(stagingBuffer.getBuffer(), tempBuffer, texture.type);
    image.transitionImageLayout(tempBuffer, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture.type, vulkanContext.getDevice());

    tempBuffer.endSingleUseCommandBuffer(queue, vulkanContext.getDevice());
    stagingBuffer.destroyBuffer(vulkanContext.getDevice());

    texture.generation++;
}

VkSamplerAddressMode VulkanBackend::convertTextureRepeatToVulkan(const String &axis, const TextureRepeat repeat) {
    switch (repeat) {
        case TEXTURE_REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TEXTURE_MIRRORED_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TEXTURE_CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case TEXTURE_CLAMP_TO_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default: {
            Logger::logWarn("Axis " + axis + " cannot be converted to repeat: " + std::to_string(repeat));
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }
}

VkFilter VulkanBackend::convertTextureFilterToVulkan(const String &op, const TextureFilter filter) {
    switch (filter) {
        case TEXTURE_FILTER_NEAREST: return VK_FILTER_NEAREST;
        case TEXTURE_FILTER_BILINEAR: return VK_FILTER_LINEAR;
        default: {
            Logger::logWarn(op + " cannot convert filter to: " + std::to_string(filter));
            return VK_FILTER_LINEAR;
        }
    }
}

VkFormat VulkanBackend::convertChannelCountToFormat(const unsigned char channelCount, VkFormat defaultFormat) {
    switch (channelCount) {
        case 1: return VK_FORMAT_R8_UNORM;
        case 2: return VK_FORMAT_R8G8_UNORM;
        case 3: return VK_FORMAT_R8G8B8_UNORM;
        case 4: return VK_FORMAT_R8G8B8A8_UNORM;
        default: return defaultFormat;
    }
}

bool VulkanBackend::acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, Texture &defaultTexture, TextureMap** maps) {
    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();
    outInstanceId = INVALID_ID_U32;

    for (unsigned int i = 0; i < 1024; i++) {
        if (backendShader->getInstanceState(i).id == INVALID_ID_U32) {
            backendShader->getInstanceState(i).id = i;
            outInstanceId = i;
            break;
        }
    }

    if (outInstanceId == INVALID_ID_U32) {
        Logger::logError("Failed to acquire instance id!");
        return false;
    }

    VulkanShaderInstanceState& instanceState = backendShader->getInstanceState(outInstanceId);
    const unsigned char samplerBindingIndex = backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].samplerBindingIndex;
    const unsigned int instanceTextureCount = backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].bindings[samplerBindingIndex].descriptorCount;
    instanceState.instanceTextureMaps.initialize(shader.getInstanceTextureCount());
    instanceState.descriptorSetState.descriptorSets.initialize(vulkanContext.getSwapchain().getImageCount());

    for (unsigned int i = 0; i < instanceTextureCount; i++) {
        TextureMap*& map = *instanceState.instanceTextureMaps.emplace();
        map = maps[i];
        if (!maps[i]->texture) {
            instanceState.instanceTextureMaps[i]->texture = &defaultTexture;
        }
    }

    const ULong size = shader.getInstanceStride();
    if (size > 0) {
        if (!backendShader->getUniformBuffer().allocate(size, instanceState.offset)) {
            Logger::logError("Failed to acquire space for instance resources!");
            return false;
        }
    }

    VulkanShaderDescriptorSetState& descriptorSetState = instanceState.descriptorSetState;

    const unsigned int bindingCount = backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].bindingCount;
    const unsigned int imageCount = vulkanContext.getSwapchain().getImageCount();
    for (unsigned int i = 0; i < bindingCount; i++) {
        descriptorSetState.descriptorStates[i].generations.initialize(imageCount);
        descriptorSetState.descriptorStates[i].ids.initialize(imageCount);

        for (unsigned int j = 0; j < imageCount; j++) {
            descriptorSetState.descriptorStates[i].generations.push(INVALID_ID_U8);
            descriptorSetState.descriptorStates[i].ids.push(INVALID_ID_U8);
        }
    }

    DynamicArray<VkDescriptorSetLayout> layouts{imageCount};
    for (unsigned int i = 0; i < imageCount; i++) {
        layouts.push(backendShader->getDescriptorSetLayout(INSTANCE_DESCRIPTOR_SET_INDEX));
    }

    VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocateInfo.descriptorPool = backendShader->getDescriptorPool();
    allocateInfo.descriptorSetCount = imageCount;
    allocateInfo.pSetLayouts = layouts.getData();
    VkResult result = vkAllocateDescriptorSets(vulkanContext.getDevice().getLogicalDevice(), &allocateInfo, instanceState.descriptorSetState.descriptorSets.getData());
    if (result != VK_SUCCESS) {
        Logger::logError("Failed to allocate instance descriptor set in shader. " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    return true;
}

bool VulkanBackend::releaseInstanceResources(const Shader &shader, const unsigned int instanceId) {
    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();
    VulkanShaderInstanceState& instanceState = backendShader->getInstanceState(instanceId);

    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    VkResult result = vkFreeDescriptorSets(vulkanContext.getDevice().getLogicalDevice(), backendShader->getDescriptorPool(), vulkanContext.getSwapchain().getImageCount(), instanceState.descriptorSetState.descriptorSets.getData());
    if (result != VK_SUCCESS) {
        Logger::logError("Error freeing instance from shader descriptor sets!");
    }

    for (VulkanDescriptorState& descriptorState : instanceState.descriptorSetState.descriptorStates) {
        descriptorState.generations.shutdown();
        descriptorState.ids.shutdown();
    }
    for (TextureMap*& textureMap : instanceState.instanceTextureMaps) {
        textureMap = nullptr;
    }

    instanceState.instanceTextureMaps.shutdown();
    instanceState.descriptorSetState.descriptorSets.shutdown();

    const bool freeResult = backendShader->getUniformBuffer().free(shader.getInstanceStride(), instanceState.offset);
    instanceState.offset = INVALID_ID_U32;
    instanceState.id = INVALID_ID_U32;

    return freeResult;
}
