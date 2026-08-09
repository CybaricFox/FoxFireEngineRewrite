/**
*   @file VulkanTypes.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "src/modules/engine/Library/FF_Math.h"

#define VULKAN_SHADER_MAX_BINDINGS 2
#define VULKAN_SHADER_MAX_PUSH_CONSTANTS 32
#define VULKAN_SHADER_MAX_STAGES 8
#define VULKAN_SHADER_MAX_ATTRIBUTES 16
#define VULKAN_SHADER_MAX_GLOBAL_TEXTURES 31
#define VULKAN_SHADER_MAX_INSTANCE_TEXTURES 31

/**
 * @brief 256 bytes. Used for camera projection and view.
 */
struct GlobalUniform {
    Mat4 projection; //64 bytes
    Mat4 view; //64 bytes
    Mat4 reserved0; //64 bytes
    Mat4 reserved1; //64 bytes
};

/**
 * @brief 256 bytes. Contains instance color.
 */
struct alignas(256) MaterialUniform {
    Vector4f diffuse; //16 bytes
    Vector4f reserved0;
    Vector4f reserved1;
    Vector4f reserved2;
};

struct VulkanShaderStage {
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
};

struct VulkanShaderStageConfig {
    VkShaderStageFlagBits stage{};
    String fileName{};
};

struct VulkanDescriptorSetConfig {
    unsigned char bindingCount = 0;
    VkDescriptorSetLayoutBinding bindings[VULKAN_SHADER_MAX_BINDINGS]{};
};

struct VulkanShaderConfig {
    unsigned char stageCount = 0;
    VulkanShaderStageConfig stages[VULKAN_SHADER_MAX_STAGES]{};
    VkDescriptorPoolSize poolSizes[2]{};
    unsigned short maxDescriptorCount = 0;
    unsigned char descriptorSetCount = 0;
    VulkanDescriptorSetConfig descriptorSets[2]{};
    VkVertexInputAttributeDescription attributes[VULKAN_SHADER_MAX_ATTRIBUTES]{};
};

struct VulkanDescriptorState {
    DynamicArray<unsigned char> generations{};
    DynamicArray<unsigned int> ids{};
};

struct VulkanShaderDescriptorSetState {
    DynamicArray<VkDescriptorSet> descriptorSets{};
    VulkanDescriptorState descriptorStates[VULKAN_SHADER_MAX_BINDINGS]{};
};

struct VulkanShaderInstanceState {
    unsigned int id = INVALID_ID_U32;
    unsigned long offset = 0;
    VulkanShaderDescriptorSetState descriptorSetState{};
    DynamicArray<Texture*> instanceTextures{};
};