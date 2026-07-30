//
// Created by cmorg on 7/15/2026.
//

#include "VulkanPipeline.h"

#include "VulkanUtils.h"
#include "src/modules/engine/Library/FF_Math.h"

void VulkanPipeline::destroyPipeline(VulkanDevice& device) {
    if (handle) {
        vkDestroyPipeline(device.getLogicalDevice(), handle, nullptr);
        handle = nullptr;
    }

    if (pipelineLayout) {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
        pipelineLayout = nullptr;
    }
}

void VulkanPipeline::bindPipeline(VulkanCommandBuffer &commandBuffer, VkPipelineBindPoint pipelineBindPoint) const {
    vkCmdBindPipeline(commandBuffer.getHandle(), pipelineBindPoint, handle);
}

bool VulkanPipeline::createPipeline(VulkanRenderpass &renderpass, unsigned int attributeCount,
    VkVertexInputAttributeDescription *attributes, unsigned int descriptorSetLayoutCount,
    VkDescriptorSetLayout*descriptorSetLayouts, unsigned int stageCount, VkPipelineShaderStageCreateInfo *shaderStages,
    VkViewport viewport, VkRect2D scissor, const bool bIsWireframe, VulkanDevice& device) {

    VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizerInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizerInfo.depthClampEnable = false;
    rasterizerInfo.rasterizerDiscardEnable = false;
    rasterizerInfo.polygonMode = bIsWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = false;
    rasterizerInfo.depthBiasConstantFactor = 0.0f;
    rasterizerInfo.depthBiasClamp = 0.0f;
    rasterizerInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleInfo.sampleShadingEnable = false;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.minSampleShading = 1.0f;
    multisampleInfo.pSampleMask = nullptr;
    multisampleInfo.alphaToCoverageEnable = false;
    multisampleInfo.alphaToOneEnable = false;

    VkPipelineDepthStencilStateCreateInfo depthStencil{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencil.depthTestEnable = true;
    depthStencil.depthWriteEnable = true;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = false;
    depthStencil.stencilTestEnable = false;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
    FF_Memory::ff_clear(&colorBlendAttachmentState, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachmentState.blendEnable = true;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendInfo.logicOpEnable = false;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachmentState;

    constexpr unsigned int dynamicStateCount = 3;
    VkDynamicState dynamicStates[dynamicStateCount] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateInfo.dynamicStateCount = dynamicStateCount;
    dynamicStateInfo.pDynamicStates = dynamicStates;

    //Vertex input
    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0; // index
    bindingDescription.stride = sizeof(Vertex3d);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //input rate is 1 per vertex

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeCount;
    vertexInputInfo.pVertexAttributeDescriptions = attributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = false;

    //Push constants
    VkPushConstantRange pushConstant;
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstant.offset = sizeof(Mat4) * 0;
    pushConstant.size = sizeof(Mat4) * 2; //= 128 bytes

    //Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutCount;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;

    VulkanUtils::vulkanCheck(vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    graphicsPipelineInfo.stageCount = stageCount;
    graphicsPipelineInfo.pStages = shaderStages;
    graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineInfo.pViewportState = &viewportState;
    graphicsPipelineInfo.pRasterizationState = &rasterizerInfo;
    graphicsPipelineInfo.pMultisampleState = &multisampleInfo;
    graphicsPipelineInfo.pDepthStencilState = &depthStencil;
    graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;
    graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
    graphicsPipelineInfo.pTessellationState = nullptr;
    graphicsPipelineInfo.layout = pipelineLayout;
    graphicsPipelineInfo.renderPass = renderpass.getHandle();
    graphicsPipelineInfo.subpass = 0;
    graphicsPipelineInfo.basePipelineHandle = nullptr;
    graphicsPipelineInfo.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(device.getLogicalDevice(), nullptr, 1, &graphicsPipelineInfo, nullptr, &handle);
    if (VulkanUtils::vulkanCheck(result)) {
        Logger::logDebug("Graphics pipeline created!");
        return true;
    }

    Logger::logError("vkCreateGraphicsPipelines failed with " + VulkanUtils::getResultAsString(result, true));
    return false;
}
