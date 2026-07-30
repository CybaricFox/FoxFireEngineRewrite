//
// Created by cmorg on 7/30/2026.
//

#include "VulkanRenderpass.h"

#include "VulkanUtils.h"

void VulkanRenderpass::beginRenderpass(VulkanCommandBuffer& commandBuffer, VkFramebuffer frameBuffer) {
    VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = x;
    beginInfo.renderArea.offset.y = y;
    beginInfo.renderArea.extent.width = w;
    beginInfo.renderArea.extent.height = h;

    VkClearValue clearValues[2];
    FF_Memory::ff_clear(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = r;
    clearValues[0].color.float32[1] = g;
    clearValues[0].color.float32[2] = b;
    clearValues[0].color.float32[3] = a;
    clearValues[1].depthStencil.depth = depth;
    clearValues[1].depthStencil.stencil = stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer.getHandle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.setState(IN_RENDER_PASS);
}

void VulkanRenderpass::endRenderpass(VulkanCommandBuffer& commandBuffer) {
    vkCmdEndRenderPass(commandBuffer.getHandle());
    commandBuffer.setState(RECORDING);
}

void VulkanRenderpass::createRenderpass(float newX, float newY, float newW, float newH, float newR,
                                                    float newG, float newB, float newA, float newDepth,
                                                    unsigned int newStencil, VkSurfaceFormatKHR &format,
                                                    VulkanDevice &device) {
    x = newX;
    y = newY;
    w = newW;
    h = newH;
    r = newR;
    g = newG;
    b = newB;
    a = newA;
    depth = newDepth;
    stencil = newStencil;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    constexpr unsigned int attachmentCount = 2;
    VkAttachmentDescription attachments[attachmentCount];

    //Color attachment
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = format.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.flags = 0;

    attachments[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    //Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = device.getDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments[1] = depthAttachment;

    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    //Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;

    //Multisampling
    subpass.pResolveAttachments = nullptr;

    //Attachements not used in this subpass but are needed for the next subpass
    subpass.preserveAttachmentCount = 0;
    subpass.pResolveAttachments = nullptr;

    //Dependencies for render pass
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    //Create render pass
    VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = attachmentCount;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;

    VulkanUtils::vulkanCheck(vkCreateRenderPass(device.getLogicalDevice(), &renderPassCreateInfo, nullptr, &handle));
}

void VulkanRenderpass::destroyRenderpass(VulkanDevice& device) {
    if (handle) {
        vkDestroyRenderPass(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }
}