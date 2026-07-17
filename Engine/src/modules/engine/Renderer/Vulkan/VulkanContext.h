//
// Created by cmorg on 7/3/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanSwapchain.h"
#include "src/modules/engine/Library/Logger.h"

struct VulkanCommandBuffer {
    VkCommandBuffer handle;
    VulkanState state;
};

struct VulkanFence {
    VkFence handle;
    bool bIsSignaled;
};

struct VulkanBuffer {
    unsigned long totalSize;
    VkBuffer handle;
    VkBufferUsageFlagBits usageFlags;
    bool bIsLocked;
    VkDeviceMemory deviceMemory;
    int memoryIndex;
    unsigned int memoryPropertyFlags;
};

class VulkanContext {
private:
    VkInstance instance = nullptr;
    VkAllocationCallbacks* allocator = nullptr;
    VkSurfaceKHR surface = nullptr;
    unsigned int frameBufferWidth = 0;
    unsigned int frameBufferHeight = 0;
    unsigned int imageIndex = 0;
    unsigned int currentFrame = 0;
    VulkanDevice device{};
    VulkanSwapchain swapchain{};
    VulkanRenderpass renderpass{};
    VulkanCommandBuffer* commandBuffers = nullptr;
    VkSemaphore* imageAvailableSemaphores = nullptr;
    VkSemaphore* queueCompleteSemaphores = nullptr;
    unsigned int inFlightFenceCount = 0;
    VulkanFence* inFlightFences = nullptr;
    VulkanFence** imagesInFlight = nullptr;
    VulkanBuffer vertexBuffer{};
    VulkanBuffer indexBuffer{};
    unsigned long geometryVertexOffset = 0;
    unsigned long geometryIndexOffset = 0;

#if ENABLE_DEBUG_LOGGING == true
    VkDebugUtilsMessengerEXT debugMessenger{};
#endif

public:
    VulkanDevice& getDevice() {return device;}
    VulkanSwapchain& getSwapchain() {return swapchain;}
    VkSurfaceKHR& getSurface() {return surface;}
    unsigned int& getCurrentFrame() { return currentFrame; }
    VkInstance& getInstance() {return instance;}
    [[nodiscard]] unsigned int getFrameBufferWidth() const { return frameBufferWidth; }
    [[nodiscard]] unsigned int getFrameBufferHeight() const { return frameBufferHeight; }
    VulkanRenderpass& getRenderpass() { return renderpass; }
    VulkanCommandBuffer* getCommandBuffers() const {return commandBuffers;}
    VulkanCommandBuffer* getCommandBuffer(const unsigned int i) const {return &commandBuffers[i];}
    VkSemaphore* getImageAvailableSemaphores() const { return imageAvailableSemaphores; }
    VkSemaphore* getQueueCompleteSemaphores() const { return queueCompleteSemaphores; }
    VulkanFence* getInFlightFences() const { return inFlightFences; }
    VulkanFence* getImagesInFlight() const { return *imagesInFlight; }
    VulkanFence& getCurrentInFlightFence() const {return inFlightFences[currentFrame];}
    VkSemaphore& getCurrentImageAvailable() const {return imageAvailableSemaphores[currentFrame];}
    unsigned int& getImageIndex() { return imageIndex; }
    VulkanCommandBuffer& getCurrentCommandBuffer() const {return commandBuffers[imageIndex];}
    VulkanFence* getCurrentImageInFlight() const {return imagesInFlight[imageIndex];}
    VkSemaphore& getCurrentQueueCompleteSemaphore() const {return queueCompleteSemaphores[imageIndex];}
    VulkanFramebuffer& getCurrentFramebuffer() const {return swapchain.getFramebuffer(imageIndex);}
    VulkanBuffer& getVertexBuffer() { return vertexBuffer; }
    VulkanBuffer& getIndexBuffer() { return indexBuffer; }

    void setWidth(const unsigned int width) {frameBufferWidth = width;}
    void setHeight(const unsigned int height) {frameBufferHeight = height;}
    void updateCurrentImageInFlight() const {imagesInFlight[imageIndex] = &inFlightFences[currentFrame];}
    void setVertexOffset(const unsigned long offset) {geometryVertexOffset = offset;}
    void setIndexOffset(const unsigned long offset) {geometryIndexOffset = offset;}

    VkDebugUtilsMessengerEXT& getDebugMessenger() {return debugMessenger;}

    void destroyContext();
    void destroyRenderpass();
    void createCommandBuffers();
    void destroyCommandBuffers();
    void destroyFences();
    void createSyncObjects();
    void destroySyncObjects();
    void clearImagesInFlight();
    void allocateAndBeginSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer);
    void allocateCommandBuffer(bool bIsPrimary, VulkanCommandBuffer& commandBuffer);
    void beginCommandBuffer(VulkanCommandBuffer& commandBuffer, bool bIsSingleUse, bool bIsRenderpassContinue, bool bIsConcurrent);
    void endSingleUseCommandBuffer(VulkanCommandBuffer& commandBuffer, VkQueue queue);
    void freeCommandBuffer(VulkanCommandBuffer& commandBuffer);
    void endCommandBuffer(VulkanCommandBuffer& commandBuffer);
};