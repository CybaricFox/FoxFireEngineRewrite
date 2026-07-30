//
// Created by cmorg on 7/11/2026.
//

#include "VulkanDevice.h"

#include <cstring>
#include <iomanip>
#include <sstream>
#include <vulkan/vulkan.h>

#include "VulkanUtils.h"
#include "src/modules/engine/Library/Logger.h"

bool VulkanDevice::createDevice(VkInstance& instance, VkSurfaceKHR& surface) {
    if (!selectPhysicalDevice(instance, surface)) {
        return false;
    }

    Logger::logInfo("Creating logical device.");
    const bool presentSharesGraphicsQueue = graphicsQueueIndex == presentQueueIndex;
    const bool transferSharesGraphicsQueue = graphicsQueueIndex == transferQueueIndex;
    unsigned int indexCount = 1;

    if (!presentSharesGraphicsQueue) {
        indexCount++;
    }
    if (!transferSharesGraphicsQueue) {
        indexCount++;
    }
    unsigned int indices[32];
    unsigned char index = 0;
    indices[index++] = graphicsQueueIndex;
    if (!presentSharesGraphicsQueue) {
        indices[index++] = presentQueueIndex;
    }
    if (!transferSharesGraphicsQueue) {
        indices[index++] = transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[32]{};
    constexpr float queuePriority[] = {1.0f, 1.0f};
    for (unsigned int i = 0; i < indexCount; i++) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;

        //This won't work on some gpus, ignore for now since it works on mine.
        if (indices[i] == graphicsQueueIndex) {
            queueCreateInfos[i].queueCount = 2;
        }

        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = nullptr;
        queueCreateInfos[i].pQueuePriorities = queuePriority;
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

    VulkanUtils::vulkanCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice));
    Logger::logInfo("Vulkan logical device created.");

    vkGetDeviceQueue(logicalDevice, graphicsQueueIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, presentQueueIndex, 0, &presentQueue);
    vkGetDeviceQueue(logicalDevice, transferQueueIndex, 0, &transferQueue);
    Logger::logInfo("Vulkan queues obtained.");

    VkCommandPoolCreateInfo poolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VulkanUtils::vulkanCheck(vkCreateCommandPool(logicalDevice, &poolCreateInfo, nullptr, &commandPool));
    Logger::logInfo("Graphics command pool created.");

    return true;
}

bool VulkanDevice::selectPhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface) {
    unsigned int deviceCount = 0;
    VulkanUtils::vulkanCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    if (deviceCount == 0) {
        Logger::logFatal("No devices were found that support Vulkan.");
        return false;
    }

    VkPhysicalDevice devices[32];
    VulkanUtils::vulkanCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, devices));

    for (unsigned int i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);

        VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(devices[i], &deviceMemoryProperties);

        //Check for local bit compat
        bool supportsDeviceLocalBit = false;
        for (unsigned int j = 0; j < deviceMemoryProperties.memoryTypeCount; j++) {
            if ((deviceMemoryProperties.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 &&
                (deviceMemoryProperties.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
                supportsDeviceLocalBit = true;
                break;
            }
        }

        //Requirements are request from engine. These are what the engine wants.
        PhysicalDeviceRequirements requirements{};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        requirements.samplerAnisotrophy = true;
        requirements.discreteGPU = true;
        requirements.extensionNames.initialize();
        requirements.extensionNames.clear();
        requirements.extensionNames.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VulkanPhysicalDeviceFamilyInfo familyInfo{};
        const bool result = physicalDeviceMeetsRequirements(
            devices[i],
            surface,
            deviceProperties,
            deviceFeatures,
            requirements,
            familyInfo,
            swapChainSupportInfo
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

            physicalDevice = devices[i];
            graphicsQueueIndex = static_cast<int>(familyInfo.graphicsFamily);
            presentQueueIndex = static_cast<int>(familyInfo.presentFamily);
            transferQueueIndex = static_cast<int>(familyInfo.transferFamily);

            physicalDeviceProperties = deviceProperties;
            physicalDeviceFeatures = deviceFeatures;
            physicalDeviceMemoryProperties = deviceMemoryProperties;
            bSupportsDeviceLocalBit = supportsDeviceLocalBit;
            break;
        }
    }

    if (!physicalDevice) {
        Logger::logError("Failed to find a valid physical device.");
        return false;
    }

    Logger::logInfo("Physical device selected.");
    return true;
}

bool VulkanDevice::physicalDeviceMeetsRequirements(VkPhysicalDevice vulkanPhysicalDevice, VkSurfaceKHR surface,
                                                    VkPhysicalDeviceProperties& deviceProperties, VkPhysicalDeviceFeatures& deviceFeatures,
                                                    PhysicalDeviceRequirements& requirements, VulkanPhysicalDeviceFamilyInfo& physicalDeviceFamilyInfo,
                                                    VulkanSwapChainSupportInfo& swapChainSupport) {
    physicalDeviceFamilyInfo.graphicsFamily = -1;
    physicalDeviceFamilyInfo.presentFamily = -1;
    physicalDeviceFamilyInfo.computeFamily = -1;
    physicalDeviceFamilyInfo.transferFamily = -1;

    if (requirements.discreteGPU) {
        if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            Logger::logInfo(String(deviceProperties.deviceName) + " is not a discrete GPU. Skipping to next device.");
            return false;
        }
    }

    unsigned int familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vulkanPhysicalDevice, &familyCount, nullptr);
    VkQueueFamilyProperties queueFamilyProperties[32];
    vkGetPhysicalDeviceQueueFamilyProperties(vulkanPhysicalDevice, &familyCount, queueFamilyProperties);

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
    bool validGraphics = !requirements.graphics;
    bool validPresent = !requirements.present;
    bool validTransfer = !requirements.transfer;
    bool validCompute = !requirements.compute;

    unsigned char minTransferScore = 255;
    for (unsigned int i = 0; i < familyCount; i++) {
        unsigned char currentScore = 0;

        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            physicalDeviceFamilyInfo.graphicsFamily = i;
            validGraphics = true;
            ++currentScore;
        }
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            physicalDeviceFamilyInfo.computeFamily = i;
            validCompute = true;
            ++currentScore;
        }
        //Since we want the dedicated transfer family on the gpu, the lower the index, the more likely it's what we're looking for.
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if (currentScore <= minTransferScore) {
                minTransferScore = currentScore;
                physicalDeviceFamilyInfo.transferFamily = i;
                validTransfer = true;
            }
        }
        VkBool32 supportsPresent = VK_FALSE;
        VulkanUtils::vulkanCheck(vkGetPhysicalDeviceSurfaceSupportKHR(vulkanPhysicalDevice, i, surface, &supportsPresent));
        if (supportsPresent) {
            physicalDeviceFamilyInfo.presentFamily = i;
            validPresent = true;
        }
    }

    std::ostringstream oss2;
    oss2 << std::setw(8) << std::to_string(physicalDeviceFamilyInfo.graphicsFamily != -1) << std::setw(3) << " | " <<
        std::setw(7) << std::to_string(physicalDeviceFamilyInfo.presentFamily != -1) << std::setw(3) << " | " <<
            std::setw(7) << std::to_string(physicalDeviceFamilyInfo.computeFamily != -1) << std::setw(3) << " | " <<
                std::setw(8) << std::to_string(physicalDeviceFamilyInfo.transferFamily != -1) << std::setw(3) << " | " <<
                    deviceProperties.deviceName;
    Logger::logInfo(oss2.str());
    oss2.clear();

    if (validGraphics && validPresent && validTransfer && validCompute) {
        Logger::logInfo(String(deviceProperties.deviceName) + " meets requirements.");
        Logger::logDebug("Graphics family index: " + std::to_string(physicalDeviceFamilyInfo.graphicsFamily));
        Logger::logDebug("Present family index: " + std::to_string(physicalDeviceFamilyInfo.presentFamily));
        Logger::logDebug("Transfer family index: " + std::to_string(physicalDeviceFamilyInfo.transferFamily));
        Logger::logDebug("Compute family index: " + std::to_string(physicalDeviceFamilyInfo.computeFamily));

        querySwapChainSupport(vulkanPhysicalDevice, surface, swapChainSupport);

        if (swapChainSupport.formatCount < 1 || swapChainSupport.presentCount < 1) {
            if (swapChainSupport.formats) {
                FF_Memory::ff_free(swapChainSupport.formats, sizeof(VkSurfaceFormatKHR) * swapChainSupport.formatCount, RENDER);
            }
            if (swapChainSupport.presentModes) {
                FF_Memory::ff_free(swapChainSupport.presentModes, sizeof(VkPresentModeKHR) * swapChainSupport.presentCount, RENDER);
            }
            Logger::logInfo("Nevermind, Swap chain is not supported by this device. Skipping to next device.");
            return false;
        }

        if (!requirements.extensionNames.isEmpty()) {
            unsigned int extensionCount = 0;
            VkExtensionProperties *availableExtensions = nullptr;
            VulkanUtils::vulkanCheck(vkEnumerateDeviceExtensionProperties(vulkanPhysicalDevice, nullptr, &extensionCount, nullptr));
            if (extensionCount != 0) {
                availableExtensions = static_cast<VkExtensionProperties *>(FF_Memory::ff_allocate(sizeof(VkExtensionProperties) * extensionCount, RENDER));
                VulkanUtils::vulkanCheck(vkEnumerateDeviceExtensionProperties(vulkanPhysicalDevice, nullptr, &extensionCount, availableExtensions));
                for (const char* extension : requirements.extensionNames) {
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

        if (requirements.samplerAnisotrophy && !deviceFeatures.samplerAnisotropy) {
            Logger::logInfo("Device does not support sampler anisotrophy, skipping.");
            return false;
        }

        return true;
    }

    return false;
}

void VulkanDevice::querySwapChainSupport(VkPhysicalDevice vulkanPhysicalDevice, VkSurfaceKHR surface, VulkanSwapChainSupportInfo& vulkanSwapchainSupportInfo) {
    VulkanUtils::vulkanCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanPhysicalDevice, surface, &vulkanSwapchainSupportInfo.capabilities));

    if (vulkanSwapchainSupportInfo.formats) {
        FF_Memory::ff_free(vulkanSwapchainSupportInfo.formats,sizeof(VkSurfaceFormatKHR) * vulkanSwapchainSupportInfo.formatCount,RENDER);
        vulkanSwapchainSupportInfo.formats = nullptr;
        vulkanSwapchainSupportInfo.formatCount = 0;
    }

    if (vulkanSwapchainSupportInfo.presentModes) {
        FF_Memory::ff_free(vulkanSwapchainSupportInfo.presentModes,sizeof(VkPresentModeKHR) * vulkanSwapchainSupportInfo.presentCount,RENDER);
        vulkanSwapchainSupportInfo.presentModes = nullptr;
        vulkanSwapchainSupportInfo.presentCount = 0;
    }

    VulkanUtils::vulkanCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanPhysicalDevice, surface, &vulkanSwapchainSupportInfo.formatCount, nullptr));
    if (vulkanSwapchainSupportInfo.formatCount > 0) {
        vulkanSwapchainSupportInfo.formats = static_cast<VkSurfaceFormatKHR *>(FF_Memory::ff_allocate(sizeof(VkSurfaceFormatKHR) * vulkanSwapchainSupportInfo.formatCount, RENDER));
        VulkanUtils::vulkanCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanPhysicalDevice, surface, &vulkanSwapchainSupportInfo.formatCount, vulkanSwapchainSupportInfo.formats));
    }

    VulkanUtils::vulkanCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanPhysicalDevice, surface, &vulkanSwapchainSupportInfo.presentCount, nullptr));
    if (vulkanSwapchainSupportInfo.presentCount > 0) {
        vulkanSwapchainSupportInfo.presentModes = static_cast<VkPresentModeKHR *>(FF_Memory::ff_allocate(sizeof(VkPresentModeKHR) * vulkanSwapchainSupportInfo.presentCount, RENDER));
        VulkanUtils::vulkanCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanPhysicalDevice, surface, &vulkanSwapchainSupportInfo.presentCount, vulkanSwapchainSupportInfo.presentModes));
    }
}