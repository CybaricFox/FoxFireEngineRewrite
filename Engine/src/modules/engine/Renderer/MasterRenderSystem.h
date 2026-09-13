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
#include "IGeometrySystem.h"
#include "IMaterialSystem.h"
#include "ITextureSystem.h"
#include "IRendererBackend.h"
#include "IRenderView.h"
#include "RenderViewSystem.h"
#include "src/defines.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraSystem.h"

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
    /** @brief pointer to the backend in use */
    IRendererBackend* backend = nullptr;

    /** @brief Pointer to the user defined texture system. */
    ITextureSystem* textureSystem = nullptr;
    /** @brief Pointer to the user defined material system. */
    IMaterialSystem* materialSystem = nullptr;
    /** @brief Pointer to the user defined Geometry system. */
    IGeometrySystem* geometrySystem = nullptr;

    ShaderSystem shaderSystem{};
    unsigned int materialShaderId = INVALID_ID_U32;
    unsigned int uiShaderId = INVALID_ID_U32;

    CameraSystem cameraSystem{};

    RenderViewSystem renderViewSystem{};

    unsigned char renderTargetCount = 0;
    unsigned int framebufferWidth = 0;
    unsigned int framebufferHeight = 0;
    Renderpass* worldRenderpass = nullptr;
    Renderpass* uiRenderpass = nullptr;
    bool bIsCurrentlyResizing = false;
    unsigned char framesSinceResizeRequested = 0;

    Texture createBlankTexture();
    void regenerateRenderTargets() const;

public:
    bool initialize(const String &appName, Platform &platform, const GameInstance &gameInstance, ResourceSystem &resources);
    bool initializeTextureSystem(unsigned int initialCapacity, ITextureSystem *system, ResourceSystem *resourceSystem);
    bool initializeMaterialSystem(MaterialSystemConfig config, IMaterialSystem *system, ResourceSystem *resourceSystem);
    bool initializeGeometrySystem(unsigned int initialCapacity, IGeometrySystem *system, ResourceSystem *resourceSystem);
    bool initializeShaderSystem(const ShaderSystemConfig &config, ResourceSystem &resources);
    bool initializeCameraSystem(const CameraSystemConfig &config, MasterEntityComponentSystem *ecsRef);
    bool initializeRenderViewSystem(const RenderViewSystemConfig &config);
    void shutdown();
    MasterRenderSystem() = default;

    [[nodiscard]] IRendererBackend* getBackend() const {return backend;}
    [[nodiscard]] Texture& getDefaultDiffuseTexture() const {return textureSystem->getDefaultDiffuseTexture();}
    [[nodiscard]] Texture& getDefaultSpecularTexture() const {return textureSystem->getDefaultSpecularTexture();}
    [[nodiscard]] Texture& getDefaultNormalTexture() const {return textureSystem->getDefaultNormalTexture();}
    [[nodiscard]] Geometry& getDefaultGeometry() const {return geometrySystem->getDefault3DGeometry();}
    [[nodiscard]] Renderpass* getRenderPass(const String &name) const {return backend->getRenderpass(name);}
    IRenderView* getRenderView(const String &name) {return renderViewSystem.getRenderView(name);}
    [[nodiscard]] unsigned int getDefaultCamera() const {return cameraSystem.getDefaultCamera();}

    [[nodiscard]] bool drawFrame(const RenderPacket &packet);
    void onResize(unsigned short width, unsigned short height);
    [[nodiscard]] Texture& acquireTexture(bool autoRelease, const String &fileName, TextureUseCase useCase) const;
    void releaseTexture(const String &name) const;
    [[nodiscard]] Geometry& acquireGeometry(GeometryConfig &config, bool autoRelease) const;
    Material& acquireMaterial(const String &name) const;
    void releaseMaterial(const String &name) const;
    bool createRenderView(const RenderViewConfig &config);
    bool buildPacket(IRenderView *renderView, MeshPacketData *meshData, RenderViewPacket &packet);

    [[nodiscard]] GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount,
        float xTile, float yTile, const String &name, const String &materialName) const;
    [[nodiscard]] GeometryConfig generateCubeConfig(float width, float height, float depth, float xTile, float yTile, const String &name, const String &materialName) const;
};
