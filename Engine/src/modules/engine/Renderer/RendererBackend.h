//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "GlobalUniform.h"
#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/Resource_Types.h"

enum RendererBackendType {
    VULKAN,
    DIRECTX
};

struct RenderPacket {
    float deltaTime;
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

    virtual bool initialize(String appName, Platform& platform, unsigned int width, unsigned int height);
    virtual bool beginFrame(float deltaTime) = 0;
    virtual bool endFrame(float deltaTime) = 0;
    virtual void resize(unsigned short width, unsigned short height) = 0;
    virtual void updateGlobalState(Mat4 projection, Mat4 view, Vector3f viewPosition, Vector4f ambientColor, int mode) = 0;
    virtual void updateEntity(const GeometryRenderData &data, Texture &defaultTexture) = 0;
    virtual void createTexture(String name, int width, int height, int channelCount, const unsigned char* pixels, bool isTransparent, Texture& outTexture) = 0;
    virtual void destroyTexture(Texture& texture) = 0;

    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};