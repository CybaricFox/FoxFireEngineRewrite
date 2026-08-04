//
// Created by cmorg on 8/2/2026.
//

#pragma once
#include "src/modules/engine/Library/ReusableArray.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Renderer/IGeometrySystem.h"
#include "src/modules/engine/Resources/Contexts.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"


struct GeometryContext {
    Geometry geometry{};
    unsigned int referenceCount = 0;
    bool bAutoRelease = false;
};

class FOXFIRE_API FoxFire_GeometrySystem final : public IGeometrySystem {
private:
    //GeometryConfig config{};
    Geometry default3DGeometry{};
    Geometry default2DGeometry{};
    ReusableArray<GeometryContext> geometries;

    Geometry& acquireGeometry(unsigned int id);
    void releaseGeometry(const Geometry& geometry);
    bool createDefaultGeometries();
    bool createGeometry(const GeometryConfig &config, Geometry& geometry);
    void destroyGeometry(Geometry& geometry);

public:
    ~FoxFire_GeometrySystem() override;

    bool initialize(unsigned initialCapacity, RendererBackend* backend, IMaterialSystem* materialSystem, ResourceSystem* resources) override;

    Geometry & getDefault3DGeometry() override {return default3DGeometry;}
    Geometry & getDefault2DGeometry() override {return default2DGeometry;}

    GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount, float xTile, float yTile, const String &name, const String &materialName) override;
    Geometry& acquireGeometry(const GeometryConfig &config, bool autoRelease) override;
};
