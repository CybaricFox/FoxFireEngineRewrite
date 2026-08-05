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

#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

enum RendererBackendType {
    VULKAN,
    DIRECTX
};

/**
 * @brief Per frame packer containing geometry data.
 */
struct RenderPacket {
    float deltaTime;
    unsigned int geometryCount;
    GeometryRenderData* geometries;
    unsigned int uiGeometryCount;
    GeometryRenderData* uiGeometries;
};

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

    virtual bool initialize(String appName, Platform &platform, unsigned int width, unsigned int height, ResourceSystem &resources) = 0;
    virtual bool beginFrame(float deltaTime) = 0;
    virtual bool endFrame(float deltaTime) = 0;
    virtual void resize(unsigned short width, unsigned short height) = 0;
    virtual void updateWorldGlobalState(Mat4 projection, Mat4 view, Vector3f viewPosition, Vector4f ambientColor, int mode) = 0;
    virtual void updateUIGlobalState(Mat4 projection, Mat4 view, int mode) = 0;
    virtual void drawGeometry(const GeometryRenderData &data, Texture &defaultTexture, Material &defaultMaterial) = 0;
    virtual void createTexture(const unsigned char* pixels, Texture& texture) = 0;
    virtual void destroyTexture(Texture& texture) = 0;
    virtual bool createMaterial(Material& material) = 0;
    virtual void destroyMaterial(Material& material) = 0;
    virtual bool createGeometry(Geometry& geometry, unsigned int vertexSize, unsigned int vertexCount, void* vertices, unsigned int indexSize, unsigned int indexCount, void* indices) = 0;
    virtual void destroyGeometry(Geometry& geometry) = 0;
    virtual void createRenderpass(RenderpassProfile profile) = 0;
    virtual void createRenderSystem(RenderSystemProfile profile) = 0;
    virtual bool beginRenderpass(unsigned char id) = 0;
    virtual bool endRenderpass(unsigned char id) = 0;


    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};