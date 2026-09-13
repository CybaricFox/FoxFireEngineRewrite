//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "IRenderView.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraSystem.h"

/**
 *  @file RenderViewSystem.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/12/2026
 *
 *  @copyright (c) 2026
 */

struct RenderViewSystemConfig {
    unsigned short maxViewCount = 0;
};

class FOXFIRE_API RenderViewSystem {
private:
    AssetMap<IRenderView*, AssetContext> assets{};
    unsigned int maxViewCount = 0;

    IRendererBackend* backendRef = nullptr;
    ShaderSystem* shaderSystemRef = nullptr;
    IMaterialSystem* materialSystemRef = nullptr;

public:
    bool initialize(RenderViewSystemConfig config, IRendererBackend *backend, ShaderSystem *shaderSystem, IMaterialSystem *materialRef);
    void shutdown();

    IRenderView *getRenderView(const String &name);

    bool createRenderView(const RenderViewConfig &config);
    void resize(unsigned int width, unsigned int height);

    bool render(IRenderView &renderView, RenderViewPacket &packet, unsigned long frameNumber, unsigned long targetIndex) const;
};