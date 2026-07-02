//
// Created by cmorg on 7/2/2026.
//

#pragma once
#include "RendererBackend.h"
#include "src/defines.h"
#include "src/modules/engine/Core/Platform.h"

class Renderer {
private:
    void onResize(short width, short height);

    bool beginFrame(float deltaTime, RendererBackend *backend);
    bool endFrame(float deltaTime, RendererBackend* backend);

public:
    bool initialize(const String& appName, PlatformState* newPlatformState, RendererBackend*& backend, const GameInstance *gameInstance);
    ~Renderer();

    bool drawFrame(const RenderPacket *packet, RendererBackend *backend);
};
