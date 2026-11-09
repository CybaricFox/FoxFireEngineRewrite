/**
*   @file MasterRenderSystem.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "GeometrySystem.h"
#include "MaterialSystem.h"
#include "TextureSystem.h"
#include "IRendererBackend.h"
#include "IRenderView.h"
#include "RenderBuffer.h"
#include "RenderViewSystem.h"
#include "src/defines.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraSystem.h"
#include "FontSystem.h"

enum RenderViewMode {
    RENDER_VIEW_DEFAULT,
    RENDER_VIEW_LIGHTING,
    RENDER_VIEW_NORMALS
};

/**
 * @brief Controls and coordinates all rendering.
 */
class FOXFIRE_API MasterRenderSystem {
private:
    static MasterRenderSystem* instance;
    /** @brief pointer to the backend in use */
    IRendererBackend* backend = nullptr;

    TextureSystem textureSystem{};

    MaterialSystem materialSystem{};

    GeometrySystem geometrySystem{};

    ShaderSystem shaderSystem{};

    CameraSystem cameraSystem{};

    RenderViewSystem renderViewSystem{};

    FontSystem fontSystem{};

    Skybox skybox{};

    unsigned char renderTargetCount = 0;
    unsigned int framebufferWidth = 0;
    unsigned int framebufferHeight = 0;
    bool bIsCurrentlyResizing = false;
    unsigned char framesSinceResizeRequested = 0;

    Texture createBlankTexture();

public:
    bool initialize(const String &appName, Platform &platform, const GameInstance &gameInstance, ResourceSystem &resources);
    bool initializeTextureSystem(unsigned int initialCapacity, ResourceSystem *resourceSystem);
    bool initializeMaterialSystem(MaterialSystemConfig config, ResourceSystem *resourceSystem);
    bool initializeGeometrySystem(unsigned int initialCapacity, ResourceSystem *resourceSystem);
    bool initializeShaderSystem(const ShaderSystemConfig &config, ResourceSystem &resources);
    bool initializeCameraSystem(const CameraSystemConfig &config, EntityComponentSystem *ecsRef);
    bool initializeRenderViewSystem(const RenderViewSystemConfig &config);
    bool initializeSkybox();
    bool initializeFontSystem(const FontSystemConfig &config, ResourceSystem *resourceSystemRef);
    void shutdown();
    MasterRenderSystem() = default;

    IRenderView* getRenderView(const String &name) {return renderViewSystem.getRenderView(name);}
    [[nodiscard]] bool isRenderSystemMultithreaded() const {return backend->isMultithreaded();}
    [[nodiscard]] unsigned int getWindowAttachmentCount() const;

    [[nodiscard]] bool drawFrame(const RenderPacket &packet);
    void onResize(unsigned short width, unsigned short height);
    bool createRenderView(const RenderViewConfig &config);
    bool buildPacket(IRenderView *renderView, void *meshData, RenderViewPacket &packet);
    void buildSkybox(const RenderPacket &packet);
    void cleanupSkybox(const RenderPacket& packet);

    static bool acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, TextureMap **maps);
    static bool createRenderBuffer(RenderBufferType type, ULong size, bool useFreeList, RenderBuffer& outBuffer);
    static bool bindRenderBuffer(RenderBuffer &buffer, ULong offset);
    static void destroyRenderBuffer(RenderBuffer& buffer);
    static bool releaseInstanceResources(const Shader &shader, unsigned int instanceId);
    static bool drawRenderBuffer(RenderBuffer &buffer, ULong offset, unsigned int elementCount, bool bindOnly);
    static bool resizeRenderBuffer(RenderBuffer& buffer, ULong newSize);
    static bool loadRenderBufferRange(RenderBuffer &buffer, ULong offset, ULong size, const void *data);
    static void destroyTexture(Texture& texture);
    static void createWritableTexture(Texture& texture);
};
