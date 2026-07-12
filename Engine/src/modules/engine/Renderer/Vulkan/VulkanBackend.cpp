//
// Created by cmorg on 7/2/2026.
//

#include "VulkanBackend.h"

#include <cassert>
#include <cstring>
#include <iomanip>

#include "../../Library/Logger.h"
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
        if (vulkanContext.getCommandBuffer(i) && vulkanContext.getCommandBuffer(i)->handle) {
            freeCommandBuffer(*vulkanContext.getCommandBuffer(i));
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
    vulkanContext.getRenderpass().w = static_cast<float>(vulkanContext.getFrameBufferWidth());
    vulkanContext.getRenderpass().h = static_cast<float>(vulkanContext.getFrameBufferHeight());
    cachedWidth = 0;
    cachedHeight = 0;
    vulkanContext.getSwapchain().finishResize();

    vulkanContext.getRenderpass().x = 0;
    vulkanContext.getRenderpass().y = 0;
    vulkanContext.getRenderpass().w = static_cast<float>(vulkanContext.getFrameBufferWidth());
    vulkanContext.getRenderpass().h = static_cast<float>(vulkanContext.getFrameBufferHeight());

    vulkanContext.getSwapchain().regenerateFramebuffers(vulkanContext.getFrameBufferWidth(), vulkanContext.getFrameBufferHeight(), vulkanContext.getRenderpass(), vulkanContext.getDevice());

    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        allocateCommandBuffer(true, *vulkanContext.getCommandBuffer(i));
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
    vulkanContext.getCurrentFrame() = (vulkanContext.getCurrentFrame() + 1) % vulkanContext.getSwapchain().getMaxFramesInFlight();
}

void VulkanBackend::beginRenderpass(VulkanCommandBuffer& commandBuffer, VkFramebuffer frameBuffer) {
    VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = vulkanContext.getRenderpass().handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = static_cast<int32_t>(vulkanContext.getRenderpass().x);
    beginInfo.renderArea.offset.y = static_cast<int32_t>(vulkanContext.getRenderpass().y);
    beginInfo.renderArea.extent.width = static_cast<int32_t>(vulkanContext.getRenderpass().w);
    beginInfo.renderArea.extent.height = static_cast<int32_t>(vulkanContext.getRenderpass().h);

    VkClearValue clearValues[2];
    FF_Memory::ff_clear(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = vulkanContext.getRenderpass().r;
    clearValues[0].color.float32[1] = vulkanContext.getRenderpass().g;
    clearValues[0].color.float32[2] = vulkanContext.getRenderpass().b;
    clearValues[0].color.float32[3] = vulkanContext.getRenderpass().a;
    clearValues[1].depthStencil.depth = vulkanContext.getRenderpass().depth;
    clearValues[1].depthStencil.stencil = vulkanContext.getRenderpass().stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer.handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.state = IN_RENDER_PASS;
}

void VulkanBackend::endRenderpass(VulkanCommandBuffer& commandBuffer) {
    vkCmdEndRenderPass(commandBuffer.handle);
    commandBuffer.state = RECORDING; //IM PRETTY SURE THIS IS SUPPOSED TO BE THE RENDER STATE
}

void VulkanBackend::allocateCommandBuffer(const bool bIsPrimary, VulkanCommandBuffer& commandBuffer) {
    FF_Memory::ff_clear(&commandBuffer, sizeof(VulkanCommandBuffer));

    VkCommandBufferAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = vulkanContext.getDevice().getCommandPool();
    allocateInfo.level = bIsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = nullptr;

    commandBuffer.state = NOT_ALLOCATED;
    VulkanUtils::vulkanCheck(vkAllocateCommandBuffers(vulkanContext.getDevice().getLogicalDevice(), &allocateInfo, &commandBuffer.handle));
    commandBuffer.state = READY;
}

void VulkanBackend::freeCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    vkFreeCommandBuffers(vulkanContext.getDevice().getLogicalDevice(), vulkanContext.getDevice().getCommandPool(), 1, &commandBuffer.handle);
    commandBuffer.handle = nullptr;
    commandBuffer.state = NOT_ALLOCATED;
}

void VulkanBackend::beginCommandBuffer(VulkanCommandBuffer& commandBuffer, const bool bIsSingleUse, const bool bIsRenderpassContinue, const bool bIsConcurrent) {
    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    beginInfo.flags = 0;
    if (bIsSingleUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (bIsRenderpassContinue) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (bIsConcurrent) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VulkanUtils::vulkanCheck(vkBeginCommandBuffer(commandBuffer.handle, &beginInfo));
    commandBuffer.state = RECORDING;
}

void VulkanBackend::endCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    VulkanUtils::vulkanCheck(vkEndCommandBuffer(commandBuffer.handle));
    commandBuffer.state = RECORDING_ENDED;
}

void VulkanBackend::updateSubmittedCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    commandBuffer.state = SUBMITTED;
}

void VulkanBackend::resetCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    commandBuffer.state = READY;
}

void VulkanBackend::allocateAndBeginSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer) {
    allocateCommandBuffer(true, commandBuffer);
    beginCommandBuffer(commandBuffer, true, false, false);
}

void VulkanBackend::endSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer, VkQueue queue) {
    endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.handle;

    VulkanUtils::vulkanCheck(vkQueueSubmit(queue, 1, &submitInfo, nullptr));

    //Wait for queue to finish since there is no fence here
    VulkanUtils::vulkanCheck(vkQueueWaitIdle(queue));

    freeCommandBuffer(commandBuffer);
}

void VulkanBackend::allocateCommandBuffers() {
    vulkanContext.createCommandBuffers();

    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        if (vulkanContext.getCommandBuffer(i)->handle) {
            freeCommandBuffer(*vulkanContext.getCommandBuffer(i));
        }
        FF_Memory::ff_clear(vulkanContext.getCommandBuffer(i), sizeof(VulkanCommandBuffer));
        allocateCommandBuffer(true, *vulkanContext.getCommandBuffer(i));
    }

    Logger::logInfo("Vulkan command buffers created and allocated.");
}

void VulkanBackend::createFence(const bool bCreateSignaled, VulkanFence& fence) {
    fence.bIsSignaled = bCreateSignaled;
    VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (fence.bIsSignaled) {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    VulkanUtils::vulkanCheck(vkCreateFence(vulkanContext.getDevice().getLogicalDevice(), &fenceCreateInfo, nullptr, &fence.handle));
}

bool VulkanBackend::waitForFence(VulkanFence& fence, const unsigned long timeout) {
    if (!fence.bIsSignaled) {
        switch (vkWaitForFences(vulkanContext.getDevice().getLogicalDevice(), 1, &fence.handle, true, timeout)) {
            case VK_SUCCESS:
                fence.bIsSignaled = true;
                return true;
            case VK_TIMEOUT:
                Logger::logWarn("Fence timed out.");
                break;
            case VK_ERROR_DEVICE_LOST:
                Logger::logError("Fence lost device.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                Logger::logError("Fence ran out of host memory.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                Logger::logError("Fence ran out of device memory.");
                break;
            default:
                Logger::logError("Fence encountered an unknown error.");
                break;
        }
    } else {
        return true;
    }

    return false;
}

void VulkanBackend::resetFence(VulkanFence& fence) {
    if (fence.bIsSignaled) {
        VulkanUtils::vulkanCheck(vkResetFences(vulkanContext.getDevice().getLogicalDevice(), 1, &fence.handle));
        fence.bIsSignaled = false;
    }
}

void VulkanBackend::resize(const unsigned short width, const unsigned short height) {
    cachedWidth = width;
    cachedHeight = height;
    vulkanContext.getSwapchain().resize();

    Logger::logInfo("Vulkan backend resized to  " + std::to_string(width) + "x" + std::to_string(height));
}

void VulkanBackend::createRenderpass(float x, float y, float w, float h, float r, float g, float b, float a, float depth, unsigned int stencil) {
    vulkanContext.getRenderpass().x = x;
    vulkanContext.getRenderpass().y = y;
    vulkanContext.getRenderpass().w = w;
    vulkanContext.getRenderpass().h = h;
    vulkanContext.getRenderpass().r = r;
    vulkanContext.getRenderpass().g = g;
    vulkanContext.getRenderpass().b = b;
    vulkanContext.getRenderpass().a = a;
    vulkanContext.getRenderpass().depth = depth;
    vulkanContext.getRenderpass().stencil = stencil;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    constexpr unsigned int attachmentCount = 2;
    VkAttachmentDescription attachments[attachmentCount];

    //Color attachment
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = vulkanContext.getSwapchain().getImageFormat().format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.flags = 0;

    attachments[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    //Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = vulkanContext.getDevice().getDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments[1] = depthAttachment;

    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.pDepthStencilAttachment = &depthAttachmentReference;

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
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;

    VulkanUtils::vulkanCheck(vkCreateRenderPass(vulkanContext.getDevice().getLogicalDevice(), &renderPassCreateInfo, nullptr, &vulkanContext.getRenderpass().handle));
}

VulkanBackend::~VulkanBackend() {
    vkDeviceWaitIdle(vulkanContext.getDevice().getLogicalDevice());

    Logger::logDebug("Destroying sync objects");
    //Destroy sync objects
    vulkanContext.destroySyncObjects();

    Logger::logDebug("Destroying command buffers.");
    //Destroy command buffers
    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        if (vulkanContext.getCommandBuffer(i)->handle) {
            freeCommandBuffer(*vulkanContext.getCommandBuffer(i));
            vulkanContext.getCommandBuffer(i)->handle = nullptr;
        }
    }
    vulkanContext.destroyCommandBuffers();

    Logger::logDebug("Destroying frame buffers.");
    for (unsigned int i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        vulkanContext.getSwapchain().destroyFramebuffer(i, vulkanContext.getDevice());
    }

    Logger::logDebug("Destroying Renderpass.");
    vulkanContext.destroyRenderpass();
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

    createRenderpass(0, 0, static_cast<float>(vulkanContext.getFrameBufferWidth()), static_cast<float>(vulkanContext.getFrameBufferHeight()), 0, 0, 0.2f, 1, 1, 0);

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
        createFence(true, vulkanContext.getInFlightFences()[i]);
    }
    for (unsigned char i = 0; i < vulkanContext.getSwapchain().getImageCount(); i++) {
        VkSemaphoreCreateInfo semCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(vulkanContext.getDevice().getLogicalDevice(), &semCreateInfo, nullptr, &vulkanContext.getQueueCompleteSemaphores()[i]);
    }

    //Set initial state to 0. This is allocated in createSyncObject().
    vulkanContext.clearImagesInFlight();

    Logger::logInfo("Vulkan renderer initialized");
    return RendererBackend::initialize(appName, platform, width, height);
}

void VulkanBackend::setVersion(const GameInstance& gameInstance) {
    majorVersion = gameInstance.config.gameVersionMajor;
    minorVersion = gameInstance.config.gameVersionMinor;
    patchVersion = gameInstance.config.gameVersionPatch;
}

bool VulkanBackend::beginFrame(const float deltaTime) {
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

    if (!waitForFence(vulkanContext.getCurrentInFlightFence(), UINT32_MAX)) {
        Logger::logWarn("In flight fence failed to wait.");
        return false;
    }

    if (!swapchainAcquireNextImageIndex(UINT32_MAX, vulkanContext.getCurrentImageAvailable(), nullptr, vulkanContext.getImageIndex())) {
        return false;
    }

    resetCommandBuffer(vulkanContext.getCurrentCommandBuffer());
    beginCommandBuffer(vulkanContext.getCurrentCommandBuffer(), false, false, false);

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

    vkCmdSetViewport(vulkanContext.getCurrentCommandBuffer().handle, 0, 1, &viewport);
    vkCmdSetScissor(vulkanContext.getCurrentCommandBuffer().handle, 0, 1, &scissor);

    vulkanContext.getRenderpass().w = static_cast<float>(vulkanContext.getFrameBufferWidth());
    vulkanContext.getRenderpass().h = static_cast<float>(vulkanContext.getFrameBufferHeight());

    beginRenderpass(vulkanContext.getCurrentCommandBuffer(), vulkanContext.getCurrentFramebuffer().handle);

    return true;
}

bool VulkanBackend::endFrame(const float deltaTime) {
    endRenderpass(vulkanContext.getCurrentCommandBuffer());
    endCommandBuffer(vulkanContext.getCurrentCommandBuffer());

    //make sure the precious fence cannot grab this new frame
    if (vulkanContext.getCurrentImageInFlight() != nullptr) {
        waitForFence(*vulkanContext.getCurrentImageInFlight(), UINT32_MAX);
    }

    vulkanContext.updateCurrentImageInFlight();
    resetFence(vulkanContext.getCurrentInFlightFence());

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanContext.getCurrentCommandBuffer().handle;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vulkanContext.getCurrentQueueCompleteSemaphore();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vulkanContext.getCurrentImageAvailable();
    constexpr VkPipelineStageFlags flags[1]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(vulkanContext.getDevice().getGraphicsQueue(), 1, &submitInfo, vulkanContext.getCurrentInFlightFence().handle);

    if (!VulkanUtils::vulkanCheck(result)) {
        Logger::logError("vkQueueSubmit failed with result: " + VulkanUtils::getResultAsString(result, true));
        return false;
    }

    updateSubmittedCommandBuffer(vulkanContext.getCurrentCommandBuffer());
    presentSwapchain();

    return true;
}