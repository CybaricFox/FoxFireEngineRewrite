//
// Created by cmorg on 7/3/2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "src/modules/engine/Library/Logger.h"

enum VulkanState {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
};

struct VulkanSwapChainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    unsigned int formatCount;
    VkSurfaceFormatKHR* formats;
    unsigned int presentCount;
    VkPresentModeKHR *presentModes;
};

struct VulkanDevice {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    int graphicsQueueIndex;
    int presentQueueIndex;
    int transferQueueIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    VulkanSwapChainSupportInfo swapChainSupportInfo;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkFormat depthFormat;
    VkCommandPool commandPool;
};

struct VulkanImage {
    VkImage handle;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    unsigned int width;
    unsigned int height;
};

struct VulkanRenderpass {
    VkRenderPass handle;
    float x;
    float y;
    float w;
    float h;
    float r;
    float g;
    float b;
    float a;
    float depth;
    unsigned int stencil;
    VulkanState state;
};

struct VulkanFramebuffer {
    VkFramebuffer handle;
    unsigned int attachmentCount;
    VkImageView* attachments;
    VulkanRenderpass* renderpass;
};

struct VulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    unsigned char maxFramesInFlight;
    VkSwapchainKHR handle;
    unsigned int imageCount;
    VkImage* images;
    VkImageView* imageViews;
    VulkanImage depthAttachment;
    VulkanFramebuffer* framebuffers;
};

struct VulkanCommandBuffer {
    VkCommandBuffer handle;
    VulkanState state;
};

struct VulkanFence {
    VkFence handle;
    bool bIsSignaled;
};

class VulkanContext {
private:
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
    unsigned int frameBufferWidth;
    unsigned int frameBufferHeight;
    unsigned int imageIndex;
    unsigned int currentFrame;
    bool bRecreateSwapchain;
    VulkanDevice device;
    VulkanSwapchain swapchain;
    VulkanRenderpass renderpass;
    VulkanCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* queueCompleteSemaphores;
    unsigned int inFlightFenceCount;
    VulkanFence* inFlightFences;
    VulkanFence** imagesInFlight;
    bool bIsSwapchainDirty = false;

#if ENABLE_DEBUG_LOGGING == true
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

public:
    [[nodiscard]] int findMemoryIndex(int typeFilter, unsigned int propertyFlags) const;

    VulkanDevice* getDevice() {return &device;}
    VulkanSwapchain* getSwapchain() {return &swapchain;}
    VkSurfaceKHR* getSurface() {return &surface;}
    unsigned int* getCurrentFrame() { return &currentFrame; }
    VkInstance* getInstance() {return &instance;}
    unsigned int getFrameBufferWidth() const { return frameBufferWidth; }
    unsigned int getFrameBufferHeight() const { return frameBufferHeight; }
    VulkanRenderpass* getRenderpass() { return &renderpass; }
    VulkanCommandBuffer* getCommandBuffers() const {return commandBuffers;}
    VulkanCommandBuffer* getCommandBuffer(const unsigned int i) const {return &commandBuffers[i];}
    VkSemaphore* getImageAvailableSemaphores() const { return imageAvailableSemaphores; }
    VkSemaphore* getQueueCompleteSemaphores() const { return queueCompleteSemaphores; }
    VulkanFence* getInFlightFences() const { return inFlightFences; }
    VulkanFence* getImagesInFlight() const { return *imagesInFlight; }
    bool isRecreatingSwapchain() const {return bRecreateSwapchain;}
    bool needsResize() const {return bIsSwapchainDirty;}
    VulkanFence* getCurrentInFlightFence() const {return &inFlightFences[currentFrame];}
    VkSemaphore* getCurrentImageAvailable() const {return &imageAvailableSemaphores[currentFrame];}
    unsigned int* getImageIndex() { return &imageIndex; }
    VulkanCommandBuffer* getCurrentCommandBuffer() const {return &commandBuffers[imageIndex];}
    VulkanFramebuffer* getCurrentFramebuffer() const {return &swapchain.framebuffers[imageIndex];}
    VulkanFence* getCurrentImageInFlight() const {return imagesInFlight[imageIndex];}
    VkSemaphore* getCurrentQueueCompleteSemaphore() const {return &queueCompleteSemaphores[imageIndex];}


    void setWidth(const unsigned int width) {frameBufferWidth = width;}
    void setHeight(const unsigned int height) {frameBufferHeight = height;}
    void resize() {bIsSwapchainDirty = true;}
    void finishResize() {bIsSwapchainDirty = false;}
    void updateCurrentImageInFlight() const {imagesInFlight[imageIndex] = &inFlightFences[currentFrame];}
    void enableRecreateSwapchain() {bRecreateSwapchain = true;}
    void finishRecreateSwapchain() {bRecreateSwapchain = false;}

    VkDebugUtilsMessengerEXT* getDebugMessenger() {return &debugMessenger;}

    void destroyContext();
    void destroyRenderpass();
    void createCommandBuffers();
    void destroyCommandBuffers();
    void destroyFences();
    void createSyncObjects();
    void clearImagesInFlight();
};