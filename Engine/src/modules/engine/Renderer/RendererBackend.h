/**
*   @file RendererBackend.h
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

#include "Shader.h"
#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

/**
 * @brief The Abstract Backend used for this application.
 */
class RendererBackend {
private:
    /** @brief pointer to platform data */
    PlatformState* platformState = nullptr;
    /** @brief Current frame*/
    unsigned long frameNumber = 0;

protected:
    RendererBackend() = default;
    ResourceSystem* resourceSystemRef = nullptr;

public:
    virtual ~RendererBackend();

    /**
     * @brief Creates the specific backend.
     * @param type Type of backend to create.
     * @param newPlatformState Platform specific data.
     * @param gameInstance Game specific data.
     * @return The specific backend to use.
     */
    static RendererBackend* create(RendererBackendType type, PlatformState& newPlatformState, const GameInstance& gameInstance);

    virtual bool getRenderpassId(String name, unsigned char& outId) = 0;

    virtual bool initialize(String appName, Platform &platform, unsigned int width, unsigned int height, ResourceSystem* resources) = 0;
    virtual bool beginFrame(float deltaTime) = 0;
    virtual bool endFrame(float deltaTime) = 0;
    virtual void resize(unsigned short width, unsigned short height) = 0;
    virtual void drawGeometry(const GeometryRenderData &data, Texture &defaultTexture, Material &defaultMaterial) = 0;
    virtual void createTexture(const unsigned char* pixels, Texture& texture) = 0;
    virtual void destroyTexture(Texture& texture) = 0;
    virtual bool createGeometry(Geometry& geometry, unsigned int vertexSize, unsigned int vertexCount, void* vertices, unsigned int indexSize, unsigned int indexCount, void* indices) = 0;
    virtual void destroyGeometry(Geometry& geometry) = 0;
    virtual void createRenderpass(RenderpassProfile profile) = 0;
    virtual bool beginRenderpass(unsigned char id) = 0;
    virtual bool endRenderpass(unsigned char id) = 0;
    virtual bool createShader(Shader& shader, unsigned char renderpassId, unsigned char stageCount, DynamicArray<String>& stageFileNames, DynamicArray<ShaderStage>& stages) = 0;
    virtual bool initializeShader(Shader& shader) = 0;
    virtual void destroyShader(Shader& shader) = 0;
    virtual bool useShader(Shader& shader) = 0;
    virtual bool bindShaderGlobals(Shader& shader) = 0;
    virtual void bindShaderInstance(Shader& shader, unsigned int instanceId) = 0;
    virtual bool setUniform(Shader& shader, ShaderUniform& uniform, void* value) = 0;
    virtual bool applyShaderGlobals(Shader& shader) = 0;
    virtual bool applyShaderInstance(Shader& shader) = 0;
    virtual bool acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, Texture &defaultTexture) = 0;
    virtual bool releaseInstanceResources(const Shader &shader, unsigned int instanceId) = 0;


    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};