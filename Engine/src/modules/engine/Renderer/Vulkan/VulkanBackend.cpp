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
unsigned int VulkanBackend::cachedWidth = 0;
unsigned int VulkanBackend::cachedHeight = 0;

constexpr unsigned int BINDING_INDEX_UBO = 0;
constexpr unsigned int BINDING_INDEX_SAMPLER = 1;
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

bool VulkanBackend::createSurface(Platform& platform) {
    return platform.createSurface();
}

//THIS FUNCTION IS CREATING ALLOCATIONS AND NOT FREEING THEM ON RESIZE!!!
//IF MEMORY LEAKS BECOME A PROBLEM, FIX THIS!!!
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

    vulkanContext.destroyFramebuffers();

    vulkanContext.getSwapchain().destroySwapchain(vulkanContext.getDevice());

    vulkanContext.getDevice().querySwapChainSupport(vulkanContext.getDevice().getPhysicalDevice(), vulkanContext.getSurface(), vulkanContext.getDevice().getSwapChainSupportInfo());
    vulkanContext.getSwapchain().detectDepthFormat(vulkanContext.getDevice());

    vulkanContext.getSwapchain().createSwapchain(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getDevice(), vulkanContext.getSurface(), vulkanContext.getCurrentFrame());

    vulkanContext.setWidth(cachedWidth);
    vulkanContext.setHeight(cachedHeight);
    for (VulkanRenderpass& renderpass : vulkanContext.getRenderpasses()) {
        renderpass.setWidth(static_cast<float>(vulkanContext.getFrameBufferWidth()));
        renderpass.setHeight(static_cast<float>(vulkanContext.getFrameBufferHeight()));
    }

    cachedWidth = 0;
    cachedHeight = 0;

    vulkanContext.getSwapchain().finishResize();

    for (VulkanRenderpass& renderpass : vulkanContext.getRenderpasses()) {
        renderpass.setRenderArea({0, 0, static_cast<float>(vulkanContext.getFrameBufferWidth()), static_cast<float>(vulkanContext.getFrameBufferHeight())});
    }

    vulkanContext.getSwapchain().regenerateFramebuffers(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getRenderpasses(), vulkanContext.getDevice());

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

bool VulkanBackend::beginRenderpass(const unsigned char renderpassId) {
    VulkanRenderpass& renderpass = vulkanContext.getRenderpass(renderpassId);
    VkFramebuffer frameBuffer = renderpass.getFramebuffer(vulkanContext.getImageIndex());
    VulkanCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer();

    renderpass.beginRenderpass(commandBuffer, frameBuffer);

    /*
    switch (renderpass.getId()) {
        case ENGINE_RENDER_PASS_WORLD: {
            shaders[0].use();
            break;
        }
        case ENGINE_RENDER_PASS_UI: {
            shaders[1].use();
            break;
        }
    }
    */

    return true;
}

bool VulkanBackend::endRenderpass(const unsigned char renderpassId) {
    VulkanRenderpass& renderpass = vulkanContext.getRenderpass(renderpassId);
    VulkanCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer();

    renderpass.endRenderpass(commandBuffer);
    return true;
}

bool VulkanBackend::createShader(Shader& shader, const unsigned char renderpassId, unsigned char stageCount, DynamicArray<String>& stageFileNames, DynamicArray<ShaderStage>& stages) {
    shader.setBackendShader(FF_Memory::ff_allocate_class<VulkanBackendShader>(sizeof(VulkanBackendShader), RENDER));

    VulkanRenderpass& renderpass = vulkanContext.getRenderpass(renderpassId);

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

    backendShader->setRenderpass(renderpass);
    backendShader->setMaxDescriptorCount(maxDescriptorAllocationCount);

    if (!backendShader->setStages(stageCount, stages, stageFileNames)) return false;

    backendShader->initializeDescriptorSets(vulkanContext.getSwapchain().getImageCount());

    backendShader->setPoolSizes();

    backendShader->createUBOConfig(BINDING_INDEX_UBO, GLOBAL_DESCRIPTOR_SET_INDEX);

    if (shader.useInstances()) {
        backendShader->createUBOConfig(BINDING_INDEX_UBO, INSTANCE_DESCRIPTOR_SET_INDEX);
    }

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

    //Process uniforms
    auto uniforms = shader.getUniforms();

    for (ShaderUniform*& uniform : uniforms) {
        if (uniform->type == SHADER_UNIFORM_TYPE_SAMPLER) {
            const unsigned int descriptorIndex = uniform->scope == SHADER_SCOPE_GLOBAL ? GLOBAL_DESCRIPTOR_SET_INDEX : INSTANCE_DESCRIPTOR_SET_INDEX;
            backendShader->setDescriptorSetConfig(descriptorIndex, BINDING_INDEX_SAMPLER);
        }
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
            shader.setUniformTexture(uniform.location, *static_cast<Texture*>(value));
        } else {
            backendShader->getInstanceState(shader.getBoundInstanceId()).instanceTextures[uniform.location] = static_cast<Texture *>(value);
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

bool VulkanBackend::applyShaderInstance(Shader &shader) {
    if (!shader.useInstances()) {
        Logger::logError("Cannot apply shader because the shader does not support instances.");
        return false;
    }

    auto* backendShader = shader.getBackendShader<VulkanBackendShader>();
    unsigned int imageIndex = vulkanContext.getImageIndex();
    VkCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer().getHandle();

    VulkanShaderInstanceState& state = backendShader->getInstanceState(shader.getBoundInstanceId());
    VkDescriptorSet& instanceDescriptor = state.descriptorSetState.descriptorSets[imageIndex];

    VkWriteDescriptorSet descriptorWrites[2]{};
    unsigned int descriptorCount = 0;
    unsigned int descriptorIndex = 0;

    unsigned char& instanceGeneration = state.descriptorSetState.descriptorStates[descriptorIndex].generations[imageIndex];

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

    VkDescriptorImageInfo imageInfos[VULKAN_SHADER_MAX_GLOBAL_TEXTURES]{};
    VkWriteDescriptorSet samplerDescriptor{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    if (backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].bindingCount > 1) {
        const unsigned int totalSamplerCount = backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].bindings[BINDING_INDEX_SAMPLER].descriptorCount;
        unsigned int updateSamplerCount = 0;
        for (unsigned int i = 0; i < totalSamplerCount; i++) {
            const Texture* texture = backendShader->getInstanceState(shader.getBoundInstanceId()).instanceTextures[i];
            auto* textureData = static_cast<VulkanTextureData *>(texture->data);
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[i].imageView = textureData->image.getImageView();
            imageInfos[i].sampler = textureData->sampler;

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
        vkUpdateDescriptorSets(vulkanContext.getDevice().getLogicalDevice(), descriptorCount, descriptorWrites, 0, nullptr);
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendShader->getPipeline().getPipelineLayout(), 1, 1, &instanceDescriptor, 0, nullptr);
    return true;
}

bool VulkanBackend::getRenderpassId(const String name, unsigned char &outId) {
    for (VulkanRenderpass& renderpass : vulkanContext.getRenderpasses()) {
        if (renderpass.getName() == name) {
            outId = renderpass.getId();
            return true;
        }
    }

    Logger::logError("Cannot find renderpass with name: " + name);
    return false;
}

void VulkanBackend::resize(const unsigned short width, const unsigned short height) {
    cachedWidth = width;
    cachedHeight = height;
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

    Logger::logDebug("Destroying Renderpasses and Framebuffers.");
    vulkanContext.destroyRenderpasses();

    Logger::logDebug("Destroying Swapchain.");
    vulkanContext.getSwapchain().destroySwapchain(vulkanContext.getDevice());
    vulkanContext.destroyContext();
}

bool VulkanBackend::initialize(const String appName, Platform& platform, const unsigned int width, const unsigned int height, ResourceSystem* resources) {
    resourceSystemRef = resources;

    vulkanContext.initializeGeometry();

    cachedWidth = width;
    cachedHeight = height;

    vulkanContext.setWidth(cachedWidth != 0 ? cachedWidth : 800);
    vulkanContext.setHeight(cachedHeight != 0 ? cachedHeight : 600);

    cachedWidth = 0;
    cachedHeight = 0;

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = appName.c_str();
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

    vulkanContext.getSwapchain().createSwapchain(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getDevice(), vulkanContext.getSurface(), vulkanContext.getCurrentFrame());

    for (VulkanRenderpass& renderpass : vulkanContext.getRenderpasses()) {
        renderpass.createRenderpass(
            {0, 0, static_cast<float>(vulkanContext.getFrameBufferWidth()), static_cast<float>(vulkanContext.getFrameBufferHeight())},
            1, 0, vulkanContext.getSwapchain().getImageFormat(), vulkanContext.getDevice());
    }

    vulkanContext.createFramebuffers();
    vulkanContext.getSwapchain().regenerateFramebuffers(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getRenderpasses(), vulkanContext.getDevice());

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

    for (VulkanRenderpass& renderpass : vulkanContext.getRenderpasses()) {
        renderpass.setWidth(static_cast<float>(vulkanContext.getFrameBufferWidth()));
        renderpass.setHeight(static_cast<float>(vulkanContext.getFrameBufferHeight()));
    }

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

    texture.data = FF_Memory::ff_allocate(sizeof(VulkanTextureData), TEXTURE);
    auto* data = static_cast<VulkanTextureData *>(texture.data);

    VkDeviceSize imageSize = texture.width * texture.height * texture.channelCount;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging{};
    staging.createBuffer(vulkanContext.getDevice(), imageSize, static_cast<VkBufferUsageFlagBits>(usage), memoryPropertyFlags, true);
    staging.loadBufferData(vulkanContext.getDevice(), 0, imageSize, pixels);

    data->image.createImage(VK_IMAGE_TYPE_2D, texture.width, texture.height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT, vulkanContext.getDevice());

    VulkanCommandBuffer tempBuffer = VulkanCommandBuffer::allocateAndBeginSingleUseCommandBuffer(vulkanContext.getDevice());
    VkQueue queue = vulkanContext.getDevice().getGraphicsQueue();

    data->image.transitionImageLayout(tempBuffer, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkanContext.getDevice());
    data->image.copyFromBuffer(staging.getBuffer(), tempBuffer);
    data->image.transitionImageLayout(tempBuffer, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vulkanContext.getDevice());
    tempBuffer.endSingleUseCommandBuffer(queue, vulkanContext.getDevice());
    staging.destroyBuffer(vulkanContext.getDevice());

    VkSamplerCreateInfo samplerCreateInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;

    VkResult result = vkCreateSampler(vulkanContext.getDevice().getLogicalDevice(), &samplerCreateInfo, nullptr, &data->sampler);
    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("Error creating texture sample: " + VulkanUtils::getResultAsString(result, true));
        return;
    }

    texture.generation++;
}

void VulkanBackend::destroyTexture(Texture &texture) {
    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    if (texture.data) {
        const auto data = static_cast<VulkanTextureData *>(texture.data);

        data->image.destroy(vulkanContext.getDevice());
        FF_Memory::ff_clear(&data->image, sizeof(VulkanImage));
        vkDestroySampler(vulkanContext.getDevice().getLogicalDevice(), data->sampler, nullptr);
        data->sampler = nullptr;

        FF_Memory::ff_free(texture.data, sizeof(VulkanTextureData), TEXTURE);
    }
}

bool VulkanBackend::createGeometry(Geometry &geometry, const unsigned int vertexSize, const unsigned int vertexCount,
    void *vertices, const unsigned int indexSize, const unsigned int indexCount, void *indices) {
    if (vertexCount == 0 || !vertices) {
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

void VulkanBackend::createRenderpass(const RenderpassProfile profile) {
    VulkanRenderpass renderpass{};
    renderpass.setName(profile.name);
    renderpass.setId(profile.id);
    renderpass.setClearFlags(profile.clearFlags);
    renderpass.setClearColor(profile.clearColor);
    renderpass.setName(profile.name);

    vulkanContext.addRenderpass(renderpass);
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

bool VulkanBackend::acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, Texture& defaultTexture) {
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
    const unsigned int instanceTextureCount = backendShader->getConfig().descriptorSets[INSTANCE_DESCRIPTOR_SET_INDEX].bindings[BINDING_INDEX_SAMPLER].descriptorCount;
    instanceState.instanceTextures.initialize(shader.getInstanceTextureCount());
    instanceState.descriptorSetState.descriptorSets.initialize(vulkanContext.getSwapchain().getImageCount());

    for (unsigned int i = 0; i < instanceTextureCount; i++) {
        instanceState.instanceTextures.push(&defaultTexture);
    }

    const unsigned long size = shader.getInstanceStride();
    if (!backendShader->getUniformBuffer().allocate(size, instanceState.offset)) {
        Logger::logError("Failed to acquire space for instance resources!");
        return false;
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

    FF_Memory::ff_clear(instanceState.descriptorSetState.descriptorStates, sizeof(VulkanDescriptorState) * VULKAN_SHADER_MAX_BINDINGS);
    instanceState.instanceTextures.shutdown();
    instanceState.descriptorSetState.descriptorSets.shutdown();

    const bool freeResult = backendShader->getUniformBuffer().free(shader.getInstanceStride(), instanceState.offset);
    instanceState.offset = INVALID_ID_U32;
    instanceState.id = INVALID_ID_U32;

    return freeResult;
}
