//
// Created by cmorg on 7/30/2026.
//

#include "VulkanRenderpass.h"

#include "VulkanBackend.h"
#include "VulkanUtils.h"

void VulkanRenderpass::beginRenderpass(VulkanCommandBuffer& commandBuffer, VkFramebuffer frameBuffer) const {
    VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = renderArea.x;
    beginInfo.renderArea.offset.y = renderArea.y;
    beginInfo.renderArea.extent.width = renderArea.z;
    beginInfo.renderArea.extent.height = renderArea.w;

    beginInfo.clearValueCount = 0;
    beginInfo.pClearValues = nullptr;

    VkClearValue clearValues[2];
    FF_Memory::ff_clear(clearValues, sizeof(VkClearValue) * 2);

    if ((clearFlags & RENDERPASS_CLEAR_COLOR) != 0) {
        FF_Memory::ff_copy(clearValues[beginInfo.clearValueCount].color.float32, clearColor.elements, sizeof(float) * 4);
        beginInfo.clearValueCount++;
    }
    if ((clearFlags & RENDERPASS_CLEAR_DEPTH) != 0) {
        FF_Memory::ff_copy(clearValues[beginInfo.clearValueCount].color.float32, clearColor.elements, sizeof(float) * 4);
        clearValues[beginInfo.clearValueCount].depthStencil.depth = depth;

        const bool doClearStencil = (clearFlags & RENDERPASS_CLEAR_STENCIL) != 0;
        clearValues[beginInfo.clearValueCount].depthStencil.stencil = doClearStencil ? stencil : 0;
        beginInfo.clearValueCount++;
    }

    beginInfo.pClearValues = beginInfo.clearValueCount > 0 ? clearValues : nullptr;

    vkCmdBeginRenderPass(commandBuffer.getHandle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.setState(IN_RENDER_PASS);
}

void VulkanRenderpass::endRenderpass(VulkanCommandBuffer& commandBuffer) {
    vkCmdEndRenderPass(commandBuffer.getHandle());
    commandBuffer.setState(RECORDING);
}

void VulkanRenderpass::createRenderpass(Vector4f render, float newDepth, unsigned int newStencil, VkSurfaceFormatKHR &format, VulkanDevice &device) {
    renderArea = render;
    depth = newDepth;
    stencil = newStencil;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    DynamicArray<VkAttachmentDescription> attachments{1};

    //Color attachment
    bool doClearColor = (clearFlags & RENDERPASS_CLEAR_COLOR) != 0;
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = format.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = doClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = bHasPreviousPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = bHasNextPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.flags = 0;

    attachments.push(colorAttachment);

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    //Depth attachment
    bool doClearDepth = (clearFlags & RENDERPASS_CLEAR_DEPTH) != 0;
    if (doClearDepth) {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = device.getDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = doClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push(depthAttachment);

        VkAttachmentReference depthAttachmentReference;
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.pDepthStencilAttachment = &depthAttachmentReference;
    } else {
        subpass.pDepthStencilAttachment = nullptr;
    }

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
    renderPassCreateInfo.attachmentCount = attachments.getLength();
    renderPassCreateInfo.pAttachments = attachments.getData();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;

    VulkanUtils::vulkanCheck(vkCreateRenderPass(device.getLogicalDevice(), &renderPassCreateInfo, nullptr, &handle));
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

void VulkanRenderpass::destroyRenderpass(VulkanDevice& device) {
    if (handle) {
        destroyFramebuffers(device);
        vkDestroyRenderPass(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }

    framebuffers.shutdown();
}
