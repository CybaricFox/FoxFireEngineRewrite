//
// Created by cmorg on 7/9/2026.
//

#include "VulkanShader.h"

#include "src/defines.h"

VulkanShader::VulkanShader() {
    String stageTypeStrings[STAGE_COUNT] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (unsigned int i = 0; i < STAGE_COUNT; i++) {

    }
}
