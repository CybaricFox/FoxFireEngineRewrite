/**
*   @file VulkanContext.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "src/modules/engine/Library/Logger.h"
#include "src/modules/engine/Library/ReusableArray.h"

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
    VulkanCommandBuffer* commandBuffers = nullptr;
    VkSemaphore* imageAvailableSemaphores = nullptr;
    VkSemaphore* queueCompleteSemaphores = nullptr;
    unsigned int inFlightFenceCount = 0;
    VkFence inFlightFences[2]{};
    DynamicArray<VkFence*> imagesInFlight{};
    VulkanBuffer vertexBuffer{};
    VulkanBuffer indexBuffer{};
    unsigned long geometryVertexOffset = 0;
    unsigned long geometryIndexOffset = 0;
    float deltaTime = 0.0f;
    ReusableArray<GeometryData> geometries{};
    DynamicArray<VulkanRenderpass> renderpasses{};

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
    VulkanRenderpass& getRenderpass(unsigned char id);
    [[nodiscard]] VulkanCommandBuffer* getCommandBuffers() const {return commandBuffers;}
    [[nodiscard]] VulkanCommandBuffer& getCommandBuffer(const unsigned int i) const {return commandBuffers[i];}
    VkSemaphore* getImageAvailableSemaphores() const { return imageAvailableSemaphores; }
    VkSemaphore* getQueueCompleteSemaphores() const { return queueCompleteSemaphores; }
    VkFence* getInFlightFences() { return inFlightFences; }
    VkFence& getFenceInFlight(const unsigned int i) {return inFlightFences[i];}
    VkFence& getCurrentInFlightFence() {return inFlightFences[currentFrame];}
    VkSemaphore& getCurrentImageAvailable() const {return imageAvailableSemaphores[currentFrame];}
    unsigned int& getImageIndex() { return imageIndex; }
    [[nodiscard]] VulkanCommandBuffer& getCurrentCommandBuffer() const {return commandBuffers[imageIndex];}
    VkFence*& getCurrentImageInFlight() {return imagesInFlight[imageIndex];}
    VkSemaphore& getCurrentQueueCompleteSemaphore() const {return queueCompleteSemaphores[imageIndex];}
    VkFramebuffer& getCurrentFramebuffer(EngineRenderpasses id);
    VulkanBuffer& getVertexBuffer() { return vertexBuffer; }
    VulkanBuffer& getIndexBuffer() { return indexBuffer; }
    [[nodiscard]] float getDeltaTime() const { return deltaTime; }
    GeometryData& getGeometry(const unsigned int id) { return geometries.get(id); }
    [[nodiscard]] unsigned long getGeometryVertexOffset() const { return geometryVertexOffset; }
    [[nodiscard]] unsigned long getGeometryIndexOffset() const { return geometryIndexOffset; }
    DynamicArray<VulkanRenderpass>& getRenderpasses() { return renderpasses; }

    void setWidth(const unsigned int width) {frameBufferWidth = width;}
    void setHeight(const unsigned int height) {frameBufferHeight = height;}
    void updateCurrentImageInFlight() {imagesInFlight[imageIndex] = &inFlightFences[currentFrame];}
    void setVertexOffset(const unsigned long offset) {geometryVertexOffset = offset;}
    void setIndexOffset(const unsigned long offset) {geometryIndexOffset = offset;}
    void setDeltaTime(const float dt) {deltaTime = dt;}
    void setCurrentFrame(const unsigned int value) {currentFrame = value;}
    unsigned int assignGeometry() {return geometries.assign();}
    void setGeometryVertexOffset(const unsigned long offset) {geometryVertexOffset = offset;}
    void setGeometryIndexOffset(const unsigned long offset) {geometryIndexOffset = offset;}
    void initializeGeometry() {geometries.initialize(0);}
    void initializeRenderpasses() {renderpasses.initialize(2);}

    VkDebugUtilsMessengerEXT& getDebugMessenger() {return debugMessenger;}

    void destroyContext();
    void createCommandBuffers();
    void destroyCommandBuffers();
    void createSyncObjects();
    void destroySyncObjects();
    void clearImagesInFlight();
    [[nodiscard]] bool isCommandBufferValid(const unsigned int i) const {return &commandBuffers[i] != nullptr;}
    void addRenderpass(VulkanRenderpass &newRenderpass);
    void createFramebuffers();
    void destroyRenderpasses();
    void destroyFramebuffers();
};
