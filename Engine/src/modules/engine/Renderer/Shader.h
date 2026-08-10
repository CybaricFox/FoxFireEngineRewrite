//
// Created by cmorg on 8/7/2026.
//

#pragma once
#include "IBackendShader.h"
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
 *  @date 8/7/2026
 *
 *  @copyright (c) 2026
 */

enum ShaderState {
    SHADER_STATE_NOT_CREATED,
    SHADER_STATE_NOT_INITIALIZED,
    SHADER_STATE_INITIALIZED
};

struct ShaderUniform {
    unsigned long offset = 0;
    unsigned short location = 0;
    unsigned short index = INVALID_ID_U16;
    unsigned short size = 0;
    unsigned char descriptorIndex = INVALID_ID_U8;
    ShaderScope scope{};
    ShaderUniformType type{};
};

struct ShaderAttribute {
    String name{};
    ShaderAttributeType type{};
    unsigned int size = 0;
};

struct ShaderAttributeConfig {
    String name{};
    unsigned char size = 0;
    ShaderAttributeType type{};
};

struct ShaderUniformConfig {
    String name{};
    unsigned char size = 0;
    unsigned int location = 0;
    ShaderUniformType type{};
    ShaderScope scope{};
};

struct ShaderConfig {
    String name{};
    bool bUseInstances = false;
    bool bUseLocals = false;
    unsigned char attributeCount = 0;
    DynamicArray<ShaderAttributeConfig> attributes{};
    unsigned char uniformCount = 0;
    DynamicArray<ShaderUniformConfig> uniforms{};
    String renderpassName{};
    unsigned char stageCount = 0;
    DynamicArray<ShaderStage> stages{};
    DynamicArray<String> stageNames{};
    DynamicArray<String> stageFileNames{};
};

class Shader {
private:
    unsigned int id = INVALID_ID_U32;
    String name{};
    bool bUseInstances = false;
    bool bUseLocals = false;
    unsigned long requiredAlignment = 0;
    unsigned long globalSize = 0;
    unsigned long globalStride = 0;
    unsigned long globalOffset = 0;
    unsigned long instanceSize = 0;
    unsigned long instanceStride = 0;
    unsigned long pushConstantSize = 0;
    unsigned long pushConstantStride = 0;
    DynamicArray<Texture*> globalTextures{};
    unsigned char instanceTextureCount = 0;
    ShaderScope boundScope{};
    unsigned int boundInstanceId = INVALID_ID_U32;
    unsigned int boundOffset = 0;
    AssetMap<ShaderUniform, AssetContext> uniforms{};
    DynamicArray<ShaderAttribute> attributes{};
    ShaderState state{};
    unsigned char pushConstantRangeCount = 0;
    MemoryRange pushConstantRanges[32]{};
    unsigned short attributeStride = 0;

    IBackendShader* backendShader = nullptr;

public:
    [[nodiscard]] IBackendShader* getBackendShader() const {return backendShader;}
    [[nodiscard]] bool useInstances() const { return bUseInstances; }
    [[nodiscard]] String const& getName() const { return name; }
    [[nodiscard]] DynamicArray<ShaderAttribute> const& getAttributes() const { return attributes; }
    ShaderAttribute& getAttribute(const unsigned int index) {return attributes[index];}
    DynamicArray<ShaderUniform*> getUniforms() {return uniforms.getAssetsAsArray();}
    [[nodiscard]] unsigned short getAttributeStride() const {return attributeStride;}
    [[nodiscard]] unsigned int getAttributeCount() const {return attributes.getLength();}
    [[nodiscard]] unsigned char getPushConstantRangeCount() const {return pushConstantRangeCount;}
    MemoryRange* getPushConstantRanges() {return pushConstantRanges;}
    [[nodiscard]] unsigned long getGlobalStride() const {return globalStride;}
    [[nodiscard]] unsigned long getInstanceStride() const {return instanceStride;}
    [[nodiscard]] unsigned long& getGlobalOffset() {return globalOffset;}
    [[nodiscard]] unsigned int getBoundInstanceId() const {return boundInstanceId;}
    Texture& getUniformTexture(const unsigned short location) {return *globalTextures[location];}
    [[nodiscard]] unsigned int getBoundOffset() const {return boundOffset;}
    [[nodiscard]] unsigned char getInstanceTextureCount() const {return instanceTextureCount;}
    [[nodiscard]] unsigned int getId() const {return id;}
    unsigned int getUniformIndex(const String &uniformName){return uniforms.getContext(uniformName)->index;}
    ShaderUniform& getUniform(const unsigned int index){return uniforms.getData().get(index);}
    [[nodiscard]] ShaderScope getBoundScope() const {return boundScope;}
    void increaseAttributeStride(const unsigned short stride) {attributeStride += stride;}
    [[nodiscard]] unsigned long getGlobalTextureCount() const {return globalTextures.getLength();}
    [[nodiscard]] unsigned int getUniformCount() const {return uniforms.getAssetCount();}
    [[nodiscard]] unsigned long getGlobalSize() const {return globalSize;}
    [[nodiscard]] unsigned long getInstanceSize() const {return instanceSize;}
    [[nodiscard]] bool useLocals() const { return bUseLocals; }
    [[nodiscard]] unsigned long getPushConstantSize() const {return pushConstantSize;}
    [[nodiscard]] ShaderState getState() const {return state;}

    template<typename T>
    requires std::derived_from<T, IBackendShader>
    T* getBackendShader() const {return reinterpret_cast<T*>(backendShader);}

    void setBackendShader(IBackendShader* shader) {backendShader = shader;}
    void setRequiredAlignment(const unsigned long value) {requiredAlignment = value;}
    void setBoundOffset(const unsigned int value) {boundOffset = value;}
    void setBoundInstanceId(const unsigned int value) {boundInstanceId = value;}
    void setUniformTexture(const unsigned short location, Texture& texture) {globalTextures[location] = &texture;}
    void setState(const ShaderState newState) {state = newState;}
    void addAttribute(const ShaderAttribute& attribute) {attributes.push(attribute);}
    void addGlobalTexture(Texture& texture) {globalTextures.push(&texture);}
    void incrementInstanceTextureCount() {++instanceTextureCount;}
    void increaseGlobalSize(const unsigned int size) {globalSize += size;}
    void increaseInstanceSize(const unsigned int size) {instanceSize += size;}

    void setGlobalStride();
    void setInstanceStride();
    bool initializeShader(const ShaderConfig &config, unsigned int newId);
    ShaderUniform* createUniform(const String &uniformName, AssetContext &context);
    void setPushConstantRange(MemoryRange range);
    bool isUniformNameValid(const String &uniformName);
    void clearName();

};