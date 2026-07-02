//
// Created by cmorg on 7/2/2026.
//

#pragma once

#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Platform.h"

enum RendererBackendType {
    VULKAN,
    DIRECTX
};

struct RenderPacket {
    float deltaTime;
};

class RendererBackend {
private:
    PlatformState* platformState;
    unsigned long frameNumber;

    void resize(short width, short height);

protected:
    RendererBackend() = default;

public:
    ~RendererBackend();

    static RendererBackend* create(RendererBackendType type, PlatformState* newPlatformState, const GameInstance *gameInstance);

    virtual bool initialize(String appName, PlatformState* newPlatformState);
    bool beginFrame(float deltaTime);
    bool endFrame(float deltaTime);

    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};