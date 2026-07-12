//
// Created by cmorg on 7/11/2026.
//

#pragma once
#include "src/modules/engine/Memory/DynamicArray.h"
#include "vulkan/vulkan.h"

struct VulkanSwapChainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    unsigned int formatCount;
    VkSurfaceFormatKHR* formats;
    unsigned int presentCount;
    VkPresentModeKHR *presentModes;
};

struct VulkanPhysicalDeviceFamilyInfo {
    unsigned int graphicsFamily;
    unsigned int presentFamily;
    unsigned int computeFamily;
    unsigned int transferFamily;
};

struct PhysicalDeviceRequirements {
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    DynamicArray<const char*> extensionNames{};
    bool samplerAnisotrophy;
    bool discreteGPU;
};

class VulkanDevice {
private:
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

    bool selectPhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);

    bool physicalDeviceMeetsRequirements(
        VkPhysicalDevice vulkanPhysicalDevice,
        VkSurfaceKHR surface,
        VkPhysicalDeviceProperties& deviceProperties,
        VkPhysicalDeviceFeatures& deviceFeatures,
        PhysicalDeviceRequirements& requirements,
        VulkanPhysicalDeviceFamilyInfo& physicalDeviceFamilyInfo,
        VulkanSwapChainSupportInfo& swapChainSupport);
public:
    bool createDevice(VkInstance &instance, VkSurfaceKHR &surface);
    void querySwapChainSupport(VkPhysicalDevice vulkanPhysicalDevice, VkSurfaceKHR surface, VulkanSwapChainSupportInfo& vulkanSwapchainSupportInfo);

    VkDevice& getLogicalDevice(){return logicalDevice;}
    VulkanSwapChainSupportInfo& getSwapChainSupportInfo(){return swapChainSupportInfo;}
    int& getGraphicsQueueIndex() {return graphicsQueueIndex;}
    int& getPresentQueueIndex() {return presentQueueIndex;}
    int& getTransferQueueIndex() {return transferQueueIndex;}
    VkCommandPool& getCommandPool() {return commandPool;}
    VkQueue& getGraphicsQueue() {return graphicsQueue;}
    VkQueue& getPresentQueue() {return presentQueue;}
    VkQueue& getTransferQueue() {return transferQueue;}
    VkPhysicalDevice& getPhysicalDevice() {return physicalDevice;}
    VkFormat& getDepthFormat() {return depthFormat;}
};