//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "RendererBackend.h"
#include "src/modules/engine/Library/Logger.h"
#include <vulkan/vulkan.h>

#include "src/modules/engine/Core/GameInstance.h"

enum VulkanTypes {

};

struct VulkanSwapChainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    unsigned int formatCount;
    VkSurfaceFormatKHR* formats;
    unsigned int presentCount;
    VkPresentModeKHR *presentModes;
};

struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks allocator;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VulkanSwapChainSupportInfo swapChainSupportInfo;
    int graphicsQueueIndex;
    int presentQueueIndex;
    int transferQueueIndex;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

#if ENABLE_DEBUG_LOGGING == true
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

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
    std::vector<const char*> extensionNames{};
    bool samplerAnisotrophy;
    bool discreteGPU;
};

class VulkanBackend final : public RendererBackend{
private:
    int majorVersion = 0;
    int minorVersion = 0;
    int patchVersion = 0;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);

    bool createDevice();
    bool createSurface(Platform* platform);

    bool selectPhysicalDevice();

    bool physicalDeviceMeetsRequirements(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        const VkPhysicalDeviceProperties *deviceProperties,
        const VkPhysicalDeviceFeatures *deviceFeatures,
        const PhysicalDeviceRequirements *requirements,
        VulkanPhysicalDeviceFamilyInfo *physicalDeviceFamilyInfo,
        VulkanSwapChainSupportInfo *swapChainSupport);

    void querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VulkanSwapChainSupportInfo* swapChainSupportInfo);

public:
    ~VulkanBackend() override;
    static VulkanContext vulkanContext;

    static void vulkanCheck(VkResult result);

    bool initialize(String appName, Platform *platform) override;

    void setVersion(const GameInstance* gameInstance);
};
