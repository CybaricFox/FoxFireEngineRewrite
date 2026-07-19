//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/Library/FF_Math.h"

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
    virtual void updateObject(Mat4 model) = 0;

    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};