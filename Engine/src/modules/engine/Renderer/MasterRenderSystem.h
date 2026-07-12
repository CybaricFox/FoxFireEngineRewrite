//
// Created by cmorg on 7/2/2026.
//

#pragma once
#include "IRenderSystem.h"
#include "RendererBackend.h"
#include "src/defines.h"
#include "src/modules/engine/Core/Platform.h"

class MasterRenderSystem {
private:
    bool beginFrame(float deltaTime, RendererBackend& backend);
    bool endFrame(float deltaTime, RendererBackend& backend);

    DynamicArray<IRenderSystem> renderSystems{};

public:
    bool initialize(const String &appName, Platform& platform, RendererBackend*& backend, const GameInstance& gameInstance, unsigned int width, unsigned int height);
    MasterRenderSystem() = default;
    ~MasterRenderSystem();

    bool drawFrame(const RenderPacket& packet, RendererBackend& backend);
    void onResize(unsigned short width, unsigned short height, RendererBackend* backend);
};
