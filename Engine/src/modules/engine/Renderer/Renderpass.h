//
// Created by cmorg on 9/11/2026.
//

#pragma once
#include "IRenderpass.h"
#include "src/defines.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/EngineTextureTypes.h"
#include "src/modules/system/FoxFire_Input/FoxFire_Events.h"

/**
 *  @file Renderpass.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/11/2026
 *
 *  @copyright (c) 2026
 */

enum RenderpassClearFlag {
    RENDERPASS_CLEAR_NONE = 0x0,
    RENDERPASS_CLEAR_COLOR = 0x1,
    RENDERPASS_CLEAR_DEPTH = 0x2,
    RENDERPASS_CLEAR_STENCIL = 0x4
};

struct RenderpassConfig {
    String name{};
    String prevName{};
    String nextName{};
    Vector4f renderArea{};
    Vector4f clearColor{};
    unsigned char clearFlags = 0;
};

struct RenderTarget {
    bool bUpdateOnResize = false;
    unsigned char attachmentCount = 0;
    DynamicArray<Texture*> attachments{};
    void* framebuffer = nullptr;
};

class Renderpass {
private:
    unsigned short id = INVALID_ID_U16;
    Vector4f renderArea{};
    Vector4f clearColor{};
    unsigned char clearFlags = 0;
    unsigned char renderTargetCount = 0;
    RenderTarget* targets = nullptr;
    IRenderpass* data = nullptr;

public:
    [[nodiscard]] IRenderpass* getData() const {return data;}
    Vector4f& getRenderArea() {return renderArea;}
    [[nodiscard]] bool hasFlag(const unsigned char flag) const {return clearFlags & flag;}
    Vector4f& getClearColor() {return clearColor;}
    [[nodiscard]] unsigned short getId() const {return id;}
    [[nodiscard]] RenderTarget& getRenderTarget(const unsigned char index) const {return targets[index];}

    void setId(const unsigned short newId) {id = newId;}
    void setClearFlags(const unsigned char newFlags) {clearFlags = newFlags;}
    void setClearColor(const Vector4f& newColor) {clearColor = newColor;}
    void setRenderArea(const Vector4f& newRenderArea) {renderArea = newRenderArea;}
    void setRenderAreaZ(const float value) {renderArea.z = value;}
    void setRenderAreaW(const float value) {renderArea.w = value;}
    void setData(IRenderpass* newData) {data = newData;}
    void setRenderTargetCount(const unsigned char newCount) {renderTargetCount = newCount;}
    void setTargets(RenderTarget* newTargets) {targets = newTargets;}
};