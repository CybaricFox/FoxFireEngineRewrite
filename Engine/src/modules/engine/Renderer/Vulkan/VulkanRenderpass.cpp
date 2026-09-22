//
// Created by cmorg on 7/30/2026.
//

#include "VulkanRenderpass.h"

#include "VulkanBackend.h"
#include "VulkanUtils.h"


void VulkanRenderpass::shutdown() {
    handle = nullptr;
    framebuffers.shutdown();
}

void VulkanRenderpass::setupFramebuffers(const unsigned int count) {
    framebuffers.initialize(count);
    for (int i = 0; i < count; ++i) {
        framebuffers.emplace();
    }
}

void VulkanRenderpass::destroyFramebuffers(VulkanDevice &device) {
    for (VkFramebuffer& framebuffer : framebuffers) {
        vkDestroyFramebuffer(device.getLogicalDevice(), framebuffer, nullptr);
    }
}
