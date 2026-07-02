//
// Created by cmorg on 7/2/2026.
//

#include "VulkanBackend.h"

#include "../Library/Logger.h"

VulkanContext VulkanBackend::vulkanContext{};

bool VulkanBackend::initialize(const String appName, PlatformState *newPlatformState) {
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
    appInfo.pEngineName = "FoxFire Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    if (const VkResult result = vkCreateInstance(&createInfo, nullptr, &vulkanContext.instance); result != VK_SUCCESS) {
        Logger::logError("vkCreateInstance failed with result: " + std::to_string(result));
        return false;
    }

    Logger::logInfo("Vulkan renderer initialized");

    return RendererBackend::initialize(appName, newPlatformState);
}

void VulkanBackend::setVersion(const GameInstance *gameInstance) {
    majorVersion = gameInstance->config.gameVersionMajor;
    minorVersion = gameInstance->config.gameVersionMinor;
    patchVersion = gameInstance->config.gameVersionPatch;
}
