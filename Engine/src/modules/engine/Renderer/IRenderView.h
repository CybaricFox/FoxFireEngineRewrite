//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "IMaterialSystem.h"
#include "Renderpass.h"
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Renderer/ShaderSystem.h"

/**
 *  @file RenderView.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/12/2026
 *
 *  @copyright (c) 2026
 */

enum RenderViewType {
    RENDER_VIEW_WORLD = 0x01,
    RENDER_VIEW_UI = 0x02
};

enum RenderViewMatrixSource {
    RENDER_VIEW_MATRIX_SOURCE_SCENE = 0x01,
    RENDER_VIEW_MATRIX_SOURCE_UI = 0x02,
    RENDER_VIEW_MATRIX_SOURCE_LIGHT = 0x03
};

enum RenderViewProjectionMatrixSource {
    RENDER_VIEW_PROJECTION_SOURCE_PERSPECTIVE = 0x01,
    RENDER_VIEW_PROJECTION_SOURCE_ORTHOGRAPHIC = 0x02
};

struct RenderViewPacket;

/**
 * @brief Per frame packer containing geometry data.
 */
struct RenderPacket {
    float deltaTime;
    unsigned short viewCount = 0;
    RenderViewPacket* views = nullptr;
};

struct RenderViewRenderpassConfig {
    String renderpassName{};
};

struct RenderViewConfig {
    String name{};
    /** @brief Only used for custom shaders. */
    String customShaderName{};
    unsigned short width = 0;
    unsigned short height = 0;
    RenderViewType type{};
    RenderViewMatrixSource viewSource{};
    RenderViewProjectionMatrixSource projectionSource{};
    unsigned char renderpassCount = 0;
    RenderViewRenderpassConfig* renderpasses = nullptr;
};

struct MeshPacketData {
    unsigned int meshCount = 0;
    unsigned int* meshes = nullptr;
};

class IRenderView {
private:
    unsigned short id = 0;
    String name{};
    RenderViewType type{};

protected:
    String customShaderName{};
    unsigned short width = 0;
    unsigned short height = 0;
    unsigned char renderpassCount = 0;
    DynamicArray<Renderpass*> renderpasses{};
    unsigned long size = 0;

    ShaderSystem* shaderSystemRef = nullptr;

public:
    virtual ~IRenderView() = default;

    virtual bool initialize(ShaderSystem* shaderRef, unsigned long newSize);
    virtual void shutdown();

    [[nodiscard]] unsigned long getSize() const {return size;}

    void setId(const unsigned short newId) {id = newId;}
    void setType(const RenderViewType newType) {type = newType;}
    void setCustomShader(const String &shaderName) {customShaderName = shaderName;}
    void setRenderpassCount(const unsigned short newCount) {renderpassCount = newCount;}
    void initializeRenderpasses() {renderpasses.initialize(renderpassCount);}
    void addRenderpass(Renderpass* renderpass) {renderpasses.push(renderpass);}
    void setName(const String newName) {name = newName;} //Do not make it a ref!

    virtual void resize(unsigned int newWidth, unsigned int newHeight) = 0;
    virtual bool buildPacket(void* data, RenderViewPacket& outPacket) = 0;
    virtual bool render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex, IRendererBackend *backendRef, IMaterialSystem* materialSystemRef) = 0;
};

struct RenderViewPacket {
    IRenderView* renderView = nullptr;
    Mat4 viewMatrix{};
    Mat4 projectionMatrix{};
    Vector3f viewPosition{};
    Vector4f ambientColor{};
    unsigned int geometryCount = 0;
    DynamicArray<GeometryRenderData> geometries{};
    String customShaderName{};
    void* data = nullptr;
};