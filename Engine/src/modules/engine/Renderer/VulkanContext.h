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
};

struct VulkanImage {
    VkImage handle;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    unsigned int width;
    unsigned int height;
};

struct VulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    unsigned char maxFramesInFlight;
    VkSwapchainKHR handle;
    unsigned int imageCount;
    VkImage* images;
    VkImageView* imageViews;
    VulkanImage depthAttachment;
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
    unsigned int* getFrameBufferWidth() { return &frameBufferWidth; }
    unsigned int* getFrameBufferHeight() { return &frameBufferHeight; }
    VulkanRenderpass* getRenderpass() { return &renderpass; }

    VkDebugUtilsMessengerEXT* getDebugMessenger() {return &debugMessenger;}

    void destroyContext();
    void destroyRenderpass();
};