//
// Created by cmorg on 11/9/2026.
//

#pragma once
#include "src/modules/engine/Renderer/IRenderView.h"

/**
 *  @file PickRenderView.h
 *  @layer System
 *  @module FoxFire_Renders
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/9/2026
 *
 *  @copyright (c) 2026
 */

struct PickRenderViewShaderData {
    Shader* shader = nullptr;
    Renderpass* renderpass = nullptr;
    unsigned short idColorLocation = INVALID_ID_U16;
    unsigned short modelLocation = INVALID_ID_U16;
    unsigned short projectionLocation = INVALID_ID_U16;
    unsigned short viewLocation = INVALID_ID_U16;
    Mat4 projection{};
    Mat4 view{};
    float nearClip = 0;
    float farClip = 0;
    float fov = 0;
};

class PickRenderView final : public IRenderView{
private:
    PickRenderViewShaderData uiShaderData{};
    PickRenderViewShaderData worldShaderData{};
    Texture colorTargetAttachmentTexture{};
    Texture depthTargetAttachmentTexture{};
    int instanceCount = 0;
    DynamicArray<bool> bIsInstanceUpdated{};
    short mouseX = 0;
    short mouseY = 0;

    void onMouseMoved(EngineInputContext context);
    void acquireShaderInstances();
    void releaseShaderInstances();

public:
    bool initialize(unsigned long newSize) override;
    void shutdown() override;

    //void getMatrices(Mat4& outView, Mat4& outProjection);

    void resize(unsigned newWidth, unsigned newHeight) override;
    bool buildPacket(void *data, RenderViewPacket &outPacket) override;
    bool render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex, IRendererBackend *backendRef) override;
    void destroyPacket(RenderViewPacket &packet) override;
    bool regenerateAttachmentTarget(unsigned passIndex, RenderTargetAttachment &attachment) override;


};