//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "GlobalUniform.h"
#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

enum RendererBackendType {
    VULKAN,
    DIRECTX
};

struct RenderPacket {
    float deltaTime;
    unsigned int geometryCount;
    GeometryRenderData* geometries;
};

class RendererBackend {
private:
    PlatformState* platformState = nullptr;
    unsigned long frameNumber = 0;

protected:
    RendererBackend() = default;

public:
    virtual ~RendererBackend();

    static RendererBackend* create(RendererBackendType type, PlatformState& newPlatformState, const GameInstance& gameInstance);

    virtual bool initialize(String appName, Platform &platform, unsigned int width, unsigned int height, ResourceSystem &resources);
    virtual bool beginFrame(float deltaTime) = 0;
    virtual bool endFrame(float deltaTime) = 0;
    virtual void resize(unsigned short width, unsigned short height) = 0;
    virtual void updateGlobalState(Mat4 projection, Mat4 view, Vector3f viewPosition, Vector4f ambientColor, int mode) = 0;
    virtual void drawGeometry(const GeometryRenderData &data, Texture &defaultTexture, Material &defaultMaterial) = 0;
    virtual void createTexture(const unsigned char* pixels, Texture& texture) = 0;
    virtual void destroyTexture(Texture& texture) = 0;
    virtual bool createMaterial(Material& material) = 0;
    virtual void destroyMaterial(Material& material) = 0;
    virtual bool createGeometry(Geometry& geometry, unsigned int vertexCount, const Vertex3d* vertices, unsigned int indexCount, const unsigned int* indices) = 0;
    virtual void destroyGeometry(Geometry& geometry) = 0;

    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};