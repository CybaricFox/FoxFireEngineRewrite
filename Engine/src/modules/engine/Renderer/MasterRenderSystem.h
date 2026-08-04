//
// Created by cmorg on 7/2/2026.
//

#pragma once
#include "IGeometrySystem.h"
#include "IMaterialSystem.h"
#include "ITextureSystem.h"
#include "RendererBackend.h"
#include "src/defines.h"
#include "src/modules/engine/Core/Platform.h"

class FOXFIRE_API MasterRenderSystem {
private:
    //Backend Renderer
    RendererBackend* backend = nullptr;
    //Camera projection
    Mat4 worldProjection{};
    Mat4 worldView{};
    Mat4 uiProjection{};
    Mat4 uiView{};
    float nearClip = 0.1f;
    float farClip = 1000.0f;

    //Texture manager
    ITextureSystem* textureSystem = nullptr;
    //Material Manager
    IMaterialSystem* materialSystem = nullptr;
    //Geometry Manager
    IGeometrySystem* geometrySystem = nullptr;

    DynamicArray<RenderpassProfile> renderpassProfiles{};
    DynamicArray<RenderSystemProfile> renderSystemProfiles{};
    bool bIsInitialized = false;

    Texture createBlankTexture();
    void createRenderpasses();
    void createRenderSystems();

public:
    bool initialize(const String &appName, Platform &platform, const GameInstance &gameInstance, unsigned int width, unsigned int height, ResourceSystem
                    &resources);
    bool initializeTextureSystem(unsigned int initialCapacity, ITextureSystem *system, ResourceSystem *resourceSystem);
    bool initializeMaterialSystem(unsigned int initialCapacity, IMaterialSystem *system, ResourceSystem *resourceSystem);
    bool initializeGeometrySystem(unsigned int initialCapacity, IGeometrySystem *system, ResourceSystem *resourceSystem);
    void shutdown();
    MasterRenderSystem() = default;
    ~MasterRenderSystem();

    [[nodiscard]] RendererBackend* getBackend() const {return backend;}
    [[nodiscard]] Texture& getDefaultTexture() const {return textureSystem->getDefaultTexture();}
    [[nodiscard]] Geometry& getDefaultGeometry() const {return geometrySystem->getDefaultGeometry();}

    void setView(const Mat4 &newView);

    [[nodiscard]] bool drawFrame(const RenderPacket &packet) const;
    void onResize(unsigned short width, unsigned short height);
    [[nodiscard]] Texture& acquireTexture(bool autoRelease, const String &fileName) const;
    void releaseTexture(const String &name) const;
    Geometry& acquireGeometry(const GeometryConfig &config, bool autoRelease) const;
    void addRenderpassProfile(const RenderpassProfile &profile);
    void addRenderSystemprofile(const RenderSystemProfile& profile);

    [[nodiscard]] GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount,
        float xTile, float yTile, const String &name, const String &materialName) const;
};
