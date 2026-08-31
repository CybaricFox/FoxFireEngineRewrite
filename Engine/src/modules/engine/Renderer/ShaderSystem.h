//
// Created by cmorg on 8/6/2026.
//

#pragma once
#include "ITextureSystem.h"
#include "Shader.h"
#include "src/modules/engine/Renderer/IRendererBackend.h"
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

/**
 * @brief Contains config data used by the shader system
 */
struct ShaderSystemConfig {
    /**
     * @brief Maximum number of shaders
     */
    unsigned short maxShaderCount = 0;
    /**
     * @brief Maximum number of uniforms per shader
     */
    unsigned char maxUniformCount = 0;
    /**
     * @brief Maximum number of textures
     */
    unsigned char maxGlobalTextures = 0;
    /**
     * @brief Maximum number of textures an instance can have
     */
    unsigned char maxInstanceTextures = 0;
};

class ShaderSystem {
private:
    /**
     * @brief Config data
     */
    ShaderSystemConfig config{};
    /**
     * @brief Asset map of shaders
     */
    AssetMap<Shader, AssetContext> assets{};
    /**
     * @brief Shader currently in use
     */
    unsigned int currentShaderId = INVALID_ID_U32;
    /**
     * @brief Pointer to the backend for reference
     */
    IRendererBackend* backendRef = nullptr;
    /**
     * @brief Pointer to the texture system for reference
     */
    ITextureSystem* textureSystemRef = nullptr;

    /**
     * @brief Sets the current shader in use
     * @param name Name of the shader
     * @return false if the shader does not exist
     */
    bool use(const String &name);

    /**
     * @brief Sets a uniform in the current shader
     * @param uniformName Name of the uniform to set
     * @param value Value to set to
     * @return false if the uniform does not exist
     */
    bool setUniform(const String &uniformName, void* value);

    /**
     * @brief Sets a sampler uniform in the current shader
     * @param samplerName Name of the sampler uniform
     * @param texture Texture to set to
     * @return false if sampler does not exist
     */
    bool setSampler(const String &samplerName, Texture& texture);
    /**
     * @brief Sets a sampler uniform in the current shader
     * @param index Index of the sampler uniform
     * @param texture Texture to set to
     * @return false if sampler does not exist
     */
    bool setSampler(unsigned short index, Texture& texture);

    /**
     * @brief Add an attribute to a shader
     * @param shader Shader to add the attribute to
     * @param attributeConfig Config data of the attribute
     * @return false on failure
     */
    bool addAttribute(Shader& shader, const ShaderAttributeConfig& attributeConfig);

    /**
     * @brief Add a sampler to a shader
     * @param shader Shader to add the uniform to
     * @param uniformConfig Config data of the sampler
     * @return false on failure
     */
    bool addSampler(Shader& shader, const ShaderUniformConfig& uniformConfig);
    /**
     * @brief Add an attribute to a shader
     * @param shader Shader to add the uniform to
     * @param uniformConfig Config data of the uniform
     * @return false on failure
     */
    bool addUniform(Shader& shader, const ShaderUniformConfig& uniformConfig);

    /**
     * @brief Add an uniform to a shader without a config
     * @param shader Shader to add the uniform to
     * @param uniformName Name of the uniform to add
     * @param size size of the uniform
     * @param type Type of uniform
     * @param scope Scope of the uniform
     * @param descriptorLocation Index in the array
     * @param isSampler Whether the uniform is actually a sampler
     * @return false on failure
     */
    bool addUniform(Shader& shader, const String &uniformName, unsigned int size, ShaderUniformType type, ShaderScope scope, unsigned int descriptorLocation, bool isSampler) const;

    /**
     * @brief Checks if the uniform has a valid name
     * @param shader Shader to use
     * @param name Name of the uniform
     * @return false if the name is not valid
     */
    bool isUniformNameValid(Shader& shader, const String &name);

    /**
     * @brief Checks if the shader is accepting uniforms
     * @param shader Shader to check
     * @return false if the shader is not accepting uniforms
     */
    bool isUniformStateValid(const Shader& shader);

    /**
     * @brief Destroys a shader
     * @param shader Shader to destroy
     */
    void destroyShader(Shader &shader) const;

    /**
     * @brief Destroys a shader
     * @param name Name of the shader to destroy
     */
    void destroyShader(const String &name);

public:
    bool initialize(ShaderSystemConfig newConfig, IRendererBackend* backend, ITextureSystem* textureSystem);
    void shutdown();

    /**
     * @brief Gets a shader
     * @param shaderId Id of the shader
     * @return Pointer to the shader
     */
    Shader *getShader(unsigned int shaderId);

    /**
     * @brief Gets a shader
     * @param shaderName Name of the shader
     * @return Pointer to the shader
     */
    Shader *getShader(const String &shaderName);

    /**
     * @brief Get the index of a uniform
     * @param shader Shader to use
     * @param uniformName Name of the uniform
     * @return Index of the uniform or INVALID_ID16 if not found
     */
    unsigned short getUniformIndex(Shader& shader, const String &uniformName);

    /**
     * @brief Sets a uniform in the current shader
     * @param index Index of the uniform
     * @param value Value to set to
     * @return false on failure
     */
    bool setUniform(unsigned short index, void* value);

    /**
     * @brief Apply global UBO
     * @return false on failure
     */
    bool applyGlobal();

    /**
     * @brief Apply instance UBO
     * @param update Whether to update the instance data (Only once per frame)
     * @return false on failure
     */
    bool applyInstance(bool update);

    /**
     * @brief
     * @param instanceId Id of the instance
     * @return false on failure
     */
    bool bindInstance(unsigned int instanceId);

    /**
     * @brief Gets a shaders id
     * @param shaderName Name of the shader
     * @return Id of the shader or INVALID_ID32 if shader not found
     */
    unsigned int getId(const String &shaderName);

    /**
     * @brief Creates a shader
     * @param shaderConfig Config data of the shader
     * @return false on failure
     */
    bool createShader(ShaderConfig &shaderConfig);

    /**
     * @brief Sets a shader to be used
     * @param shaderId Id of the shader
     * @return false on failure
     */
    bool use(unsigned int shaderId);
};