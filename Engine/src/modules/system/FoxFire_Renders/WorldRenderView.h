//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "src/modules/engine/Renderer/IMaterialSystem.h"
#include "src/modules/engine/Renderer/IRenderView.h"

/**
 *  @file WorldRenderView.h
*   @layer System
 *  @module FoxFire_Renders
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/12/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API WorldRenderView final : public IRenderView {
private:
    unsigned int shaderId = 0;
    float fov = 0;
    float nearClip = 0;
    float farClip = 0;
    Mat4 projectionMatrix{};
    unsigned int worldCamera = INVALID_ID_U32;
    Vector4f ambientColor{};
    unsigned int renderMode = 0;

public:
    bool initialize(ShaderSystem* shaderRef, unsigned long newSize) override;
    void shutdown() override;

    void setCamera(const unsigned int camera) {worldCamera = camera;}
    void subToEvent(Event<void>& event) {event.subscribe([this]() {onDebugEvent();});}

    void onDebugEvent();

    void resize(unsigned newWidth, unsigned newHeight) override;
    bool buildPacket(void *data, RenderViewPacket &outPacket) override;
    bool render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex, IRendererBackend* backendRef, IMaterialSystem* materialSystemRef) override;
};