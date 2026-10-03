//
// Created by cmorg on 8/7/2026.
//

#include "Shader.h"

bool Shader::initializeShader(const ShaderConfig &config, const unsigned int newId) {
    id = newId;
    if (id == INVALID_ID_U32) {
        Logger::logError("Failed to find a free slot to create a new shader.");
        return false;
    }

    state = SHADER_STATE_NOT_CREATED;
    name = config.name;
    pushConstantRangeCount = 0;
    FF_Memory::ff_clear(pushConstantRanges, sizeof(MemoryRange) * 32);
    boundInstanceId = INVALID_ID_U32;
    attributeStride = 0;
    globalTextureMaps.shutdown();
    globalTextureMaps.initialize(0);
    uniforms.shutdown();
    uniforms.initialize(0);
    attributes.shutdown();
    attributes.initialize(0);
    globalSize = 0;
    instanceSize = 0;
    pushConstantStride = 128;
    pushConstantSize = 0;

    return true;
}

ShaderUniform * Shader::createUniform(const String &uniformName, AssetContext& context) {
    return uniforms.createAsset(uniformName, context);
}

void Shader::setPushConstantRange(const MemoryRange range) {
    pushConstantRanges[pushConstantRangeCount] = range;
    pushConstantRangeCount++;
    pushConstantSize += range.size;
}

bool Shader::isUniformNameValid(const String &uniformName) {
    return uniforms.getContext(uniformName).index == INVALID_ID_U32;
}

void Shader::clearName() {
    if (!name.empty()) name.clear();
}

void Shader::setTextureMap(const unsigned int index, TextureMap *map) {
    globalTextureMaps[index] = map;
}

void Shader::destroyTextureMaps() {
    for (TextureMap* map : globalTextureMaps) {
        FF_Memory::ff_free(map, sizeof(TextureMap), RENDER);
    }
    globalTextureMaps.shutdown();
}

void Shader::ensureAttributeAlignment() {
    if (attributeStride % 16 != 0) {
        Logger::logWarn("Attribute stride for shader " + name + " is not a multiple of 16! Forcing alignment!");

        unsigned short stride = attributeStride;
        while (stride > 16) {
            stride -= 16;
        }
        Logger::logWarn("Old alignment: " + std::to_string(attributeStride));
        attributeStride += (16 - stride);
        Logger::logWarn("New alignment: " + std::to_string(attributeStride));
    }
}

void Shader::setGlobalStride() {
    globalStride = alignMemory(globalSize, requiredAlignment);
}

void Shader::setInstanceStride() {
    instanceStride = alignMemory(instanceSize, requiredAlignment);
}
