//
// Created by cmorg on 7/2/2026.
//

#include "VulkanBackend.h"

#include <cassert>
#include <cstring>
#include <iomanip>

#include "../../Library/Logger.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Memory/FF_Memory.h"

VulkanContext VulkanBackend::vulkanContext{};
unsigned int VulkanBackend::cachedWidth = 0;
unsigned int VulkanBackend::cachedHeight = 0;

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
    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        vulkanContext.getSwapchain().destroyFramebuffer(i, vulkanContext.getDevice());
    }

    vulkanContext.getSwapchain().destroySwapchain(vulkanContext.getDevice());

    vulkanContext.getDevice().querySwapChainSupport(vulkanContext.getDevice().getPhysicalDevice(), vulkanContext.getSurface(), vulkanContext.getDevice().getSwapChainSupportInfo());
    vulkanContext.getSwapchain().detectDepthFormat(vulkanContext.getDevice());

    vulkanContext.getSwapchain().createSwapchain(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getDevice(), vulkanContext.getSurface(), vulkanContext.getCurrentFrame());

    vulkanContext.setWidth(cachedWidth);
    vulkanContext.setHeight(cachedHeight);
    vulkanContext.getRenderpass().setWidth(static_cast<float>(vulkanContext.getFrameBufferWidth()));
    vulkanContext.getRenderpass().setHeight(static_cast<float>(vulkanContext.getFrameBufferHeight()));
    cachedWidth = 0;
    cachedHeight = 0;
    vulkanContext.getSwapchain().finishResize();

    vulkanContext.getRenderpass().setX(0);
    vulkanContext.getRenderpass().setY(0);
    vulkanContext.getRenderpass().setWidth(static_cast<float>(vulkanContext.getFrameBufferWidth()));
    vulkanContext.getRenderpass().setHeight(static_cast<float>(vulkanContext.getFrameBufferHeight()));

    vulkanContext.getSwapchain().regenerateFramebuffers(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getRenderpass(), vulkanContext.getDevice());

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

void VulkanBackend::uploadRangeOfData(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer &buffer, const unsigned long offset, const unsigned long size, const void *data) {
    constexpr VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer stagingBuffer{};
    stagingBuffer.createBuffer(vulkanContext.getDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true);

    stagingBuffer.loadBufferData(vulkanContext.getDevice(), offset, size, data);
    stagingBuffer.copyBufferData(vulkanContext.getDevice(), pool, fence, queue, stagingBuffer.getBuffer(), 0, buffer.getBuffer(), offset, size);
    stagingBuffer.destroyBuffer(vulkanContext.getDevice());
}

void VulkanBackend::resize(const unsigned short width, const unsigned short height) {
    cachedWidth = width;
    cachedHeight = height;
    vulkanContext.getSwapchain().resize();

    Logger::logInfo("Vulkan backend resized to  " + std::to_string(width) + "x" + std::to_string(height));
}

VulkanBackend::~VulkanBackend() {
    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    //Destroy shader modules
    vulkanShader.destroy(vulkanContext);

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

    Logger::logDebug("Destroying frame buffers.");
    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        vulkanContext.getSwapchain().destroyFramebuffer(i, vulkanContext.getDevice());
    }

    Logger::logDebug("Destroying Renderpass.");
    vulkanContext.getRenderpass().destroyRenderpass(vulkanContext.getDevice());
    Logger::logDebug("Destroying Swapchain.");
    vulkanContext.getSwapchain().destroySwapchain(vulkanContext.getDevice());
    vulkanContext.destroyContext();
}

bool VulkanBackend::initialize(const String appName, Platform& platform, const unsigned int width, const unsigned int height) {
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

    vulkanContext.getRenderpass().createRenderpass(0, 0,
        static_cast<float>(vulkanContext.getFrameBufferWidth()),
        static_cast<float>(vulkanContext.getFrameBufferHeight()), 0, 0, 0.2f, 1,
        1, 0, vulkanContext.getSwapchain().getImageFormat(), vulkanContext.getDevice());

    vulkanContext.getSwapchain().createFramebuffers();

    vulkanContext.getSwapchain().regenerateFramebuffers(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getRenderpass(), vulkanContext.getDevice());

    Logger::logInfo("Creating and allocating command buffers");
    allocateCommandBuffers();

    //Sync objects
    Logger::logInfo("Creating fences");
    vulkanContext.createSyncObjects();

    for (unsigned char i = 0; i < vulkanContext.getSwapchain().getMaxFramesInFlight(); i++) {
        VkSemaphoreCreateInfo semCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(vulkanContext.getDevice().getLogicalDevice(), &semCreateInfo, nullptr, &vulkanContext.getImageAvailableSemaphores()[i]);
        vulkanContext.getFenceInFlight(i).createFence(vulkanContext.getDevice(), true);
    }
    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        VkSemaphoreCreateInfo semCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(vulkanContext.getDevice().getLogicalDevice(), &semCreateInfo, nullptr, &vulkanContext.getQueueCompleteSemaphores()[i]);
    }

    //Set initial state to 0. This is allocated in createSyncObject().
    vulkanContext.clearImagesInFlight();

    //Create shader system
    if (!vulkanShader.initialize(vulkanContext)) {
        Logger::logError("Failed to initialize Vulkan shaders.");
        return false;
    }

    vulkanShader.createBuffers(vulkanContext);

    //TEMPORARY TEST CODE
    constexpr unsigned int vertexCount = 4;
    Vertex3d vertices[vertexCount];
    FF_Memory::ff_clear(vertices, sizeof(Vertex3d) * vertexCount);

    constexpr float f = 10.0f;

    vertices[0].position = {-0.5 * f, -0.5f * f, 0};
    vertices[0].textureCoordinate = {0, 0};
    vertices[1].position = {0.5f * f, 0.5f * f, 0};
    vertices[1].textureCoordinate = {1, 1};
    vertices[2].position = {-0.5 * f, 0.5f * f, 0};
    vertices[2].textureCoordinate = {0, 1};
    vertices[3].position = {0.5 * f, -0.5f * f, 0};
    vertices[3].textureCoordinate = {1, 0};

    constexpr unsigned int indexCount = 6;
    constexpr unsigned int indices[indexCount] = {0, 1, 2, 0, 3, 1};

    uploadRangeOfData(vulkanContext.getDevice().getCommandPool(), nullptr, vulkanContext.getDevice().getGraphicsQueue(), vulkanContext.getVertexBuffer(), 0, sizeof(Vertex3d) * vertexCount, vertices);
    uploadRangeOfData(vulkanContext.getDevice().getCommandPool(), nullptr, vulkanContext.getDevice().getGraphicsQueue(), vulkanContext.getIndexBuffer(), 0, sizeof(unsigned int) * indexCount, indices);

    unsigned int id = 0;
    if (!vulkanShader.aquireResources(vulkanContext, id)) {
        Logger::logError("Failed to aquire shader resource.");
        return false;
    }
    //END TEST CODE

    Logger::logInfo("Vulkan renderer initialized");
    return RendererBackend::initialize(appName, platform, width, height);
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

    if (!vulkanContext.getCurrentInFlightFence().waitForFence(vulkanContext.getDevice(), UINT32_MAX)) {
        Logger::logWarn("In flight fence failed to wait.");
        return false;
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

    vulkanContext.getRenderpass().setWidth(static_cast<float>(vulkanContext.getFrameBufferWidth()));
    vulkanContext.getRenderpass().setHeight(static_cast<float>(vulkanContext.getFrameBufferHeight()));

    vulkanContext.getRenderpass().beginRenderpass(vulkanContext.getCurrentCommandBuffer(), vulkanContext.getCurrentFramebuffer().handle);

    return true;
}

bool VulkanBackend::endFrame(const float deltaTime) {
    vulkanContext.getRenderpass().endRenderpass(vulkanContext.getCurrentCommandBuffer());
    vulkanContext.getCurrentCommandBuffer().endCommandBuffer();

    //make sure the precious fence cannot grab this new frame
    if (vulkanContext.getCurrentImageInFlight() != nullptr) {
        vulkanContext.getCurrentImageInFlight()->waitForFence(vulkanContext.getDevice(), UINT32_MAX);
    }

    vulkanContext.updateCurrentImageInFlight();
    vulkanContext.getCurrentInFlightFence().resetFence(vulkanContext.getDevice());

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanContext.getCurrentCommandBuffer().getHandle();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vulkanContext.getCurrentQueueCompleteSemaphore();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vulkanContext.getCurrentImageAvailable();
    constexpr VkPipelineStageFlags flags[1]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(vulkanContext.getDevice().getGraphicsQueue(), 1, &submitInfo, vulkanContext.getCurrentInFlightFence().getFence());

    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("vkQueueSubmit failed with result: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    vulkanContext.getCurrentCommandBuffer().updateSubmittedCommandBuffer();
    presentSwapchain();

    return true;
}

void VulkanBackend::updateGlobalState(const Mat4 projection, const Mat4 view, Vector3f viewPosition, Vector4f ambientColor, int mode) {
    vulkanShader.use(vulkanContext);

    vulkanShader.getUBO().projection = projection;
    vulkanShader.getUBO().view = view;

    vulkanShader.updateGlobalState(vulkanContext);
}

void VulkanBackend::updateEntity(const GeometryRenderData &data, Texture& defaultTexture) {
    vulkanShader.updateEntity(vulkanContext, data, defaultTexture);

    //TEST CODE
    VulkanCommandBuffer& commandBuffer = vulkanContext.getCurrentCommandBuffer();
    vulkanShader.use(vulkanContext);
    constexpr VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer.getHandle(), 0, 1, &vulkanContext.getVertexBuffer().getBuffer(), offsets);
    vkCmdBindIndexBuffer(commandBuffer.getHandle(), vulkanContext.getIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer.getHandle(), 6, 1, 0, 0, 0);
    //END TEST CODE
}

void VulkanBackend::createTexture(String name, const int width, const int height, const int channelCount, const unsigned char *pixels, const bool isTransparent, Texture &outTexture) {
    outTexture.width = width;
    outTexture.height = height;
    outTexture.channelCount = channelCount;
    outTexture.generation = INVALID_ID;

    outTexture.data = FF_Memory::ff_allocate(sizeof(VulkanTextureData), TEXTURE);
    auto* data = static_cast<VulkanTextureData *>(outTexture.data);

    VkDeviceSize imageSize = width * height * channelCount;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging{};
    staging.createBuffer(vulkanContext.getDevice(), imageSize, static_cast<VkBufferUsageFlagBits>(usage), memoryPropertyFlags, true);
    staging.loadBufferData(vulkanContext.getDevice(), 0, imageSize, pixels);

    data->image.createImage(VK_IMAGE_TYPE_2D, width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
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

    outTexture.bIsTransparent = isTransparent;
    outTexture.generation++;
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
