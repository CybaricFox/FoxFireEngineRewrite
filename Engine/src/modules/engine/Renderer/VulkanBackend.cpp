//
// Created by cmorg on 7/2/2026.
//

#include "VulkanBackend.h"

#include <cassert>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../Library/Logger.h"

VulkanContext VulkanBackend::vulkanContext{};

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

bool VulkanBackend::createDevice() {
    if (!selectPhysicalDevice()) {
        return false;
    }

    Logger::logInfo("Creating logical device.");
    bool presentSharesGraphicsQueue = vulkanContext.graphicsQueueIndex == vulkanContext.presentQueueIndex;
    bool transferSharesGraphicsQueue = vulkanContext.graphicsQueueIndex == vulkanContext.transferQueueIndex;
    unsigned int indexCount = 1;

    if (!presentSharesGraphicsQueue) {
        indexCount++;
    }
    if (!transferSharesGraphicsQueue) {
        indexCount++;
    }
    unsigned int indices[indexCount];
    unsigned char index = 0;
    indices[index++] = vulkanContext.graphicsQueueIndex;
    if (!presentSharesGraphicsQueue) {
        indices[index++] = vulkanContext.presentQueueIndex;
    }
    if (!transferSharesGraphicsQueue) {
        indices[index++] = vulkanContext.transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[indexCount];
    for (unsigned int i = 0; i < indexCount; i++) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;
        if (indices[i] == vulkanContext.graphicsQueueIndex) {
            queueCreateInfos[i].queueCount = 2;
        }
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = nullptr;
        float queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.queueCreateInfoCount = indexCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 1;
    const auto extensionNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    createInfo.ppEnabledExtensionNames = &extensionNames;

    vulkanCheck(vkCreateDevice(vulkanContext.physicalDevice, &createInfo, nullptr, &vulkanContext.logicalDevice));
    Logger::logInfo("Vulkan logical device created.");

    vkGetDeviceQueue(vulkanContext.logicalDevice, vulkanContext.graphicsQueueIndex, 0, &vulkanContext.graphicsQueue);
    vkGetDeviceQueue(vulkanContext.logicalDevice, vulkanContext.presentQueueIndex, 0, &vulkanContext.presentQueue);
    vkGetDeviceQueue(vulkanContext.logicalDevice, vulkanContext.transferQueueIndex, 0, &vulkanContext.transferQueue);
    Logger::logInfo("Vulkan queues obtained.");

    return true;
}

bool VulkanBackend::createSurface(Platform* platform) {
    return platform->createSurface();
}

bool VulkanBackend::selectPhysicalDevice() {
    unsigned int deviceCount = 0;
    vulkanCheck(vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, nullptr));
    if (deviceCount == 0) {
        Logger::logFatal("No devices were found that support Vulkan.");
        return false;
    }

    VkPhysicalDevice devices[deviceCount];
    vulkanCheck(vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, devices));

    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);

        //Requirements are request from engine. These are what the engine wants.
        PhysicalDeviceRequirements requirements{};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        requirements.samplerAnisotrophy = true;
        requirements.discreteGPU = true;
        requirements.extensionNames.clear();
        requirements.extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VulkanPhysicalDeviceFamilyInfo familyInfo{};
        const bool result = physicalDeviceMeetsRequirements(
            device,
            vulkanContext.surface,
            &deviceProperties,
            &deviceFeatures,
            &requirements,
            &familyInfo,
            &vulkanContext.swapChainSupportInfo
            );

        if (result) {
            Logger::logInfo("Selected device: " + String(deviceProperties.deviceName));
            switch (deviceProperties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    Logger::logInfo("GPU type is unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    Logger::logInfo("GPU type is integated GPU.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    Logger::logInfo("GPU type is discrete GPU.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    Logger::logInfo("GPU type is virtual GPU.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    Logger::logInfo("GPU type is CPU.");
                    break;
            }

            Logger::logInfo("GPU driver version: " +
                std::to_string(VK_VERSION_MAJOR(deviceProperties.driverVersion)) +
                "." +
                std::to_string(VK_VERSION_MINOR(deviceProperties.driverVersion)) +
                "." +
                std::to_string(VK_VERSION_PATCH(deviceProperties.driverVersion)));

            Logger::logInfo("Vulkan API version: " +
                std::to_string(VK_VERSION_MAJOR(deviceProperties.apiVersion)) +
                "." +
                std::to_string(VK_VERSION_MINOR(deviceProperties.apiVersion)) +
                "." +
                std::to_string(VK_VERSION_PATCH(deviceProperties.apiVersion)));

            for (unsigned int i = 0; i < deviceMemoryProperties.memoryHeapCount; i++) {
                float memorySizeGB = static_cast<float>(deviceMemoryProperties.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f;
                if (deviceMemoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    std::ostringstream oss;
                    oss << "Local GPU memory: " << std::setprecision(2) << memorySizeGB << "GB";
                    Logger::logInfo(oss.str());
                } else {
                    std::ostringstream oss;
                    oss << "Shared system memory: " << std::setprecision(2) << memorySizeGB << "GB";
                    Logger::logInfo(oss.str());
                }
            }

            vulkanContext.physicalDevice = device;
            vulkanContext.graphicsQueueIndex = static_cast<int>(familyInfo.graphicsFamily);
            vulkanContext.presentQueueIndex = static_cast<int>(familyInfo.presentFamily);
            vulkanContext.transferQueueIndex = static_cast<int>(familyInfo.transferFamily);

            vulkanContext.physicalDeviceProperties = deviceProperties;
            vulkanContext.physicalDeviceFeatures = deviceFeatures;
            vulkanContext.physicalDeviceMemoryProperties = deviceMemoryProperties;
            break;
        }
    }

    if (!vulkanContext.physicalDevice) {
        Logger::logError("Failed to find a valid physical device.");
        return false;
    }

    Logger::logInfo("Physical device selected.");
    return true;
}

bool VulkanBackend::physicalDeviceMeetsRequirements(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties *deviceProperties, const VkPhysicalDeviceFeatures *deviceFeatures,
    const PhysicalDeviceRequirements *requirements, VulkanPhysicalDeviceFamilyInfo *physicalDeviceFamilyInfo,
    VulkanSwapChainSupportInfo *swapChainSupport) {
    physicalDeviceFamilyInfo->graphicsFamily = -1;
    physicalDeviceFamilyInfo->presentFamily = -1;
    physicalDeviceFamilyInfo->computeFamily = -1;
    physicalDeviceFamilyInfo->transferFamily = -1;

    if (requirements->discreteGPU) {
        if (deviceProperties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            Logger::logInfo(String(deviceProperties->deviceName) + " is not a discrete GPU. Skipping to next device.");
            return false;
        }
    }

    unsigned int familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    VkQueueFamilyProperties queueFamilyProperties[familyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, queueFamilyProperties);

    std::ostringstream oss1;
    oss1 << std::setw(8) << "Graphics" << std::setw(3) << " | " <<
        std::setw(7) << "Present" << std::setw(3) << " | " <<
            std::setw(7) << "Compute" << std::setw(3) << " | " <<
                std::setw(8) << "Transfer" << std::setw(3) << " | " <<
                    "Name";
    Logger::logInfo(oss1.str());

    //Whether this device meets requirements
    //If the requirement is set to false, set the validity to true since we can ignore it
    //If the requirement is set to true, set the validity to false so the below loop can check if it is valid
    bool validGraphics = !requirements->graphics;
    bool validPresent = !requirements->present;
    bool validTransfer = !requirements->transfer;
    bool validCompute = !requirements->compute;

    unsigned char minTransferScore = 255;
    for (unsigned int i = 0; i < familyCount; i++) {
        unsigned char currentScore = 0;

        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            physicalDeviceFamilyInfo->graphicsFamily = i;
            validGraphics = true;
            ++currentScore;
        }
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            physicalDeviceFamilyInfo->computeFamily = i;
            validCompute = true;
            ++currentScore;
        }
        //Since we want the dedicated transfer family on the gpu, the lower the index, the more likely it's what we're looking for.
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if (currentScore <= minTransferScore) {
                minTransferScore = currentScore;
                physicalDeviceFamilyInfo->transferFamily = i;
                validTransfer = true;
            }
        }
        VkBool32 supportsPresent = VK_FALSE;
        vulkanCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent));
        if (supportsPresent) {
            physicalDeviceFamilyInfo->presentFamily = i;
            validPresent = true;
        }
    }

    std::ostringstream oss2;
    oss2 << std::setw(8) << std::to_string(physicalDeviceFamilyInfo->graphicsFamily != -1) << std::setw(3) << " | " <<
        std::setw(7) << std::to_string(physicalDeviceFamilyInfo->presentFamily != -1) << std::setw(3) << " | " <<
            std::setw(7) << std::to_string(physicalDeviceFamilyInfo->computeFamily != -1) << std::setw(3) << " | " <<
                std::setw(8) << std::to_string(physicalDeviceFamilyInfo->transferFamily != -1) << std::setw(3) << " | " <<
                    deviceProperties->deviceName;
    Logger::logInfo(oss2.str());
    oss2.clear();

    if (validGraphics && validPresent && validTransfer && validCompute) {
        Logger::logInfo(String(deviceProperties->deviceName) + " meets requirements.");
        Logger::logDebug("Graphics family index: " + std::to_string(physicalDeviceFamilyInfo->graphicsFamily));
        Logger::logDebug("Present family index: " + std::to_string(physicalDeviceFamilyInfo->presentFamily));
        Logger::logDebug("Transfer family index: " + std::to_string(physicalDeviceFamilyInfo->transferFamily));
        Logger::logDebug("Compute family index: " + std::to_string(physicalDeviceFamilyInfo->computeFamily));

        querySwapChainSupport(physicalDevice, surface, swapChainSupport);

        if (swapChainSupport->formatCount < 1 || swapChainSupport->presentCount < 1) {
            if (swapChainSupport->formats) {
                FF_Memory::ff_free(swapChainSupport->formats, sizeof(VkSurfaceFormatKHR) * swapChainSupport->formatCount, RENDER);
            }
            if (swapChainSupport->presentModes) {
                FF_Memory::ff_free(swapChainSupport->presentModes, sizeof(VkPresentModeKHR) * swapChainSupport->presentCount, RENDER);
            }
            Logger::logInfo("Nevermind, Swap chain is not supported by this device. Skipping to next device.");
            return false;
        }

        if (!requirements->extensionNames.empty()) {
            unsigned int extensionCount = 0;
            VkExtensionProperties *availableExtensions = nullptr;
            vulkanCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
            if (extensionCount != 0) {
                availableExtensions = static_cast<VkExtensionProperties *>(FF_Memory::ff_allocate(sizeof(VkExtensionProperties) * extensionCount, RENDER));
                vulkanCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions));
                for (const char* extension : requirements->extensionNames) {
                    bool found = false;
                    for (unsigned int i = 0; i < extensionCount; i++) {
                        if (strcmp(extension, availableExtensions[i].extensionName) == 0) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        Logger::logInfo(String(extension) + " not found. Skipping device.");
                        FF_Memory::ff_free(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, RENDER);
                        return false;
                    }
                }
            }
            FF_Memory::ff_free(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, RENDER);
        }

        if (requirements->samplerAnisotrophy && !deviceFeatures->samplerAnisotropy) {
            Logger::logInfo("Device does not support sampler anisotrophy, skipping.");
            return false;
        }

        return true;
    }

    return false;
}

void VulkanBackend::querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VulkanSwapChainSupportInfo *swapChainSupportInfo) {
    vulkanCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainSupportInfo->capabilities));

    vulkanCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &swapChainSupportInfo->formatCount, nullptr));
    if (swapChainSupportInfo->formatCount != 0) {
        if (!swapChainSupportInfo->formats) {
            swapChainSupportInfo->formats = static_cast<VkSurfaceFormatKHR*>(FF_Memory::ff_allocate(sizeof(VkSurfaceFormatKHR) * swapChainSupportInfo->formatCount, RENDER));
        }
        vulkanCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &swapChainSupportInfo->formatCount, swapChainSupportInfo->formats));
    }

    vulkanCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &swapChainSupportInfo->presentCount, nullptr));
    if (swapChainSupportInfo->presentCount != 0) {
        if (!swapChainSupportInfo->presentModes) {
            swapChainSupportInfo->presentModes = static_cast<VkPresentModeKHR *>(FF_Memory::ff_allocate(sizeof(VkPresentModeKHR) * swapChainSupportInfo->presentCount, RENDER));
        }
        vulkanCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &swapChainSupportInfo->presentCount, swapChainSupportInfo->presentModes));
    }
}

VulkanBackend::~VulkanBackend() {
    Logger::logInfo("Releasing Vulkan device resources");
    vulkanContext.graphicsQueue = nullptr;
    vulkanContext.presentQueue = nullptr;
    vulkanContext.transferQueue = nullptr;

    if (vulkanContext.logicalDevice) {
        vkDestroyDevice(vulkanContext.logicalDevice, nullptr);
        vulkanContext.logicalDevice = nullptr;
    }

    vulkanContext.physicalDevice = nullptr;

    if (vulkanContext.swapChainSupportInfo.formats) {
        FF_Memory::ff_free(vulkanContext.swapChainSupportInfo.formats, sizeof(VkSurfaceFormatKHR) * vulkanContext.swapChainSupportInfo.formatCount, RENDER);
        vulkanContext.swapChainSupportInfo.formats = nullptr;
        vulkanContext.swapChainSupportInfo.formatCount = 0;
    }

    if (vulkanContext.swapChainSupportInfo.presentModes) {
        FF_Memory::ff_free(vulkanContext.swapChainSupportInfo.presentModes, sizeof(VkPresentModeKHR) * vulkanContext.swapChainSupportInfo.presentCount, RENDER);
        vulkanContext.swapChainSupportInfo.presentModes = nullptr;
        vulkanContext.swapChainSupportInfo.presentCount = 0;
    }

    FF_Memory::ff_clear(&vulkanContext.swapChainSupportInfo.capabilities, sizeof(vulkanContext.swapChainSupportInfo.capabilities));

    vulkanContext.graphicsQueueIndex = -1;
    vulkanContext.presentQueueIndex = -1;
    vulkanContext.transferQueueIndex = -1;

    Logger::logDebug("Destroying Vulkan debugger.");
    if (vulkanContext.debugMessenger) {
        const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanContext.instance, "vkDestroyDebugUtilsMessengerEXT"));
        function(vulkanContext.instance, vulkanContext.debugMessenger, nullptr);
    }

    Logger::logDebug("Destroying Vulkan surface.");
    vkDestroySurfaceKHR(vulkanContext.instance, vulkanContext.surface, nullptr);

    Logger::logDebug("Destroying Vulkan instance.");
    vkDestroyInstance(vulkanContext.instance, nullptr);
}

void VulkanBackend::vulkanCheck(VkResult result) {
    assert(result == VK_SUCCESS);
}

bool VulkanBackend::initialize(const String appName, Platform* platform) {
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
    appInfo.pEngineName = "FoxFire Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

    //Get required extensions
    std::vector<const char*> requiredExtensions{};
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    platform->getRequiredExtensions(requiredExtensions);
#if ENABLE_DEBUG_LOGGING == true
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    Logger::logDebug("Required Extensions: ");
    for (const String& extension : requiredExtensions) {
        Logger::logDebug(extension);
    }
#endif

    std::vector<const char*> validationLayers{};
    unsigned int layerCount = 0;

#if ENABLE_DEBUG_LOGGING == true
    Logger::logDebug("Debug mode enable. Starting validation layers.");

    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    layerCount = validationLayers.size();
    unsigned int availableLayerCount = 0;
    vulkanCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
    VkLayerProperties availableLayers[availableLayerCount];
    vulkanCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

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

    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = validationLayers.data();

    vulkanCheck(vkCreateInstance(&createInfo, nullptr, &vulkanContext.instance));
    Logger::logDebug("Vulkan Instance Created Successfully.");

#if ENABLE_DEBUG_LOGGING == true
    Logger::logDebug("Creating Vulkan debugger.");
    constexpr unsigned int logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

    const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanContext.instance, "vkCreateDebugUtilsMessengerEXT"));
    assert(function);
    vulkanCheck(function(vulkanContext.instance, &debugCreateInfo, nullptr, &vulkanContext.debugMessenger));
    Logger::logDebug("Vulkan debugger created successfully.");
#endif

    //Create surface
    Logger::logDebug("Creating Vulkan surface.");
    if (!createSurface(platform)) {
        Logger::logFatal("Failed to create surface for Vulkan.");
        return false;
    }
    Logger::logDebug("Created Vulkan surface successfully.");

    //Create device
    if (!createDevice()) {
        Logger::logFatal("Failed to create Vulkan device.");
        return false;
    }

    Logger::logInfo("Vulkan renderer initialized");
    return RendererBackend::initialize(appName, platform);
}

void VulkanBackend::setVersion(const GameInstance *gameInstance) {
    majorVersion = gameInstance->config.gameVersionMajor;
    minorVersion = gameInstance->config.gameVersionMinor;
    patchVersion = gameInstance->config.gameVersionPatch;
}
