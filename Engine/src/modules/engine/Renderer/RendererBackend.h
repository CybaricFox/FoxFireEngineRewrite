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


protected:
    RendererBackend() = default;

public:
    virtual ~RendererBackend();

    static RendererBackend* create(RendererBackendType type, PlatformState* newPlatformState, const GameInstance *gameInstance);

    virtual bool initialize(String appName, Platform* platform, unsigned int width, unsigned int height);
    virtual bool beginFrame(float deltaTime) = 0;
    virtual bool endFrame(float deltaTime) = 0;
    virtual void resize(unsigned short width, unsigned short height) = 0;

    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};