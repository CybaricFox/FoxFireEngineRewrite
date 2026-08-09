//
// Created by cmorg on 8/6/2026.
//

#pragma once
#include "ITextureSystem.h"
#include "Shader.h"
#include "src/modules/engine/Renderer/RendererBackend.h"
#include "src/defines.h"
#include "src/modules/engine/Library/AssetMap.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

/**
 *  @file Shader.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/6/2026
 *
 *  @copyright (c) 2026
 */

#define DEFAULT_MATERIAL_SHADER_NAME "Fox_Fire_Material_Shader"
#define DEFAULT_UI_SHADER_NAME "Fox_Fire_UI_Shader"

struct ShaderSystemConfig {
    unsigned short maxShaderCount = 0;
    unsigned char maxUniformCount = 0;
    unsigned char maxGlobalTextures = 0;
    unsigned char maxInstanceTextures = 0;
};

class ShaderSystem {
private:
    ShaderSystemConfig config{};
    AssetMap<Shader, AssetContext> assets{};
    unsigned int currentShaderId = INVALID_ID_U32;
    RendererBackend* backendRef = nullptr;
    ITextureSystem* textureSystemRef = nullptr;

    bool use(const String &name);
    bool setUniform(const String &uniformName, void* value);
    bool setSampler(const String &samplerName, Texture& texture);
    bool setSampler(unsigned short index, Texture& texture);
    bool addAttribute(Shader& shader, const ShaderAttributeConfig& attributeConfig);
    bool addSampler(Shader& shader, const ShaderUniformConfig& uniformConfig);
    bool addUniform(Shader& shader, const ShaderUniformConfig& uniformConfig);
    bool addUniform(Shader& shader, const String &uniformName, unsigned int size, ShaderUniformType type, ShaderScope scope, unsigned int descriptorLocation, bool isSampler) const;
    bool isUniformNameValid(Shader& shader, const String &name);
    bool isUniformStateValid(const Shader& shader);
    void destroyShader(Shader &shader) const;
    void destroyShader(const String &name);

public:
    bool initialize(ShaderSystemConfig newConfig, RendererBackend* backend, ITextureSystem* textureSystem);
    void shutdown();

    Shader *getShader(unsigned int shaderId);
    Shader *getShader(const String &shaderName);
    unsigned short getUniformIndex(Shader& shader, const String &uniformName);
    bool setUniform(unsigned short index, void* value);
    bool applyGlobal();
    bool applyInstance();
    bool bindInstance(unsigned int instanceId);
    unsigned int getId(const String &shaderName);
    bool createShader(ShaderConfig &shaderConfig);
    bool use(unsigned int shaderId);
};