//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "Renderpass.h"
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Renderer/ShaderSystem.h"
#include "src/modules/engine/UserInterface/UIText.h"

/**
 *  @file IRenderView.h
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
    RENDER_VIEW_UI = 0x02,
    RENDER_VIEW_SKYBOX = 0x03,
    RENDER_VIEW_PICK = 0x04
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
    float deltaTime = 0;
    unsigned short viewCount = 0;
    RenderViewPacket* views = nullptr;
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
    RenderpassConfig* renderpasses = nullptr;
};

struct MeshPacketData {
    unsigned int meshCount = 0;
    unsigned int* meshes = nullptr;
};

struct UIPacketData {
    MeshPacketData meshData{};
    unsigned int textCount = 0;
    UIText** texts = nullptr;
};

struct SkyboxPacketData {
    Skybox* skybox = nullptr;
};

struct PickPacketData {
    MeshPacketData worldMeshData{};
    unsigned int worldGeometryCount = 0;
    MeshPacketData uiMeshData{};
    unsigned int uiGeometryCount = 0;
    unsigned int textCount = 0;
    UIText** texts = nullptr;
};

class IRenderView {
private:
    unsigned short id = 0;
    RenderViewType type{};

protected:
    String customShaderName{};
    unsigned short width = 0;
    unsigned short height = 0;
    unsigned char renderpassCount = 0;
    Renderpass* renderpasses = nullptr;
    unsigned long size = 0;
    String name{};

public:
    virtual ~IRenderView() = default;

    virtual bool initialize(unsigned long newSize);
    virtual void shutdown();

    [[nodiscard]] unsigned long getSize() const {return size;}
    Renderpass& getRenderpass(const unsigned short index) const {return renderpasses[index];}
    unsigned char getRenderpassCount() const {return renderpassCount;}
    String getName() const {return name;}

    void setId(const unsigned short newId) {id = newId;}
    void setType(const RenderViewType newType) {type = newType;}
    void setCustomShader(const String &shaderName) {customShaderName = shaderName;}
    void setRenderpassCount(const unsigned short newCount) {renderpassCount = newCount;}
    void initializeRenderpasses() {renderpasses = FF_Memory::ff_allocate_recursive<Renderpass>(ARRAY, renderpassCount);}
    void setName(const String &newName) {name = newName;}

    virtual void resize(unsigned int newWidth, unsigned int newHeight) = 0;
    virtual bool buildPacket(void* data, RenderViewPacket& outPacket) = 0;
    virtual bool render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex, IRendererBackend *backendRef) = 0;
    virtual void destroyPacket(RenderViewPacket& packet) = 0;
    virtual bool regenerateAttachmentTarget(unsigned int passIndex, RenderTargetAttachment& attachment) = 0;
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