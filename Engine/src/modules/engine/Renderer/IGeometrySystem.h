//
// Created by cmorg on 8/2/2026.
//

#pragma once

#include <foxfire_export.h>

#include "IMaterialSystem.h"
#include "RendererBackend.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

#define DEFAULT_GEOMETRY_NAME "default"

struct GeometryConfig {
    unsigned int vertexSize = 0;
    unsigned int vertexCount = 0;
    void* vertices = nullptr;
    unsigned int indexSize = 0;
    unsigned int indexCount = 0;
    void* indices = nullptr;
    String name{};
    String materialName{};
    String materialPath{};
};

class FOXFIRE_API IGeometrySystem {
protected:
    RendererBackend* backendRef = nullptr;
    IMaterialSystem* materialSystemRef = nullptr;
    ResourceSystem* resourceSystemRef = nullptr;

public:
    virtual ~IGeometrySystem();

    virtual bool initialize(unsigned int initialCapacity, RendererBackend* backend, IMaterialSystem* materialSystem, ResourceSystem* resources);

    virtual Geometry& getDefault3DGeometry() = 0;
    virtual Geometry& getDefault2DGeometry() = 0;

    virtual GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount, float xTile, float yTile, const String &name, const String &materialName) = 0;
    virtual Geometry& acquireGeometry(const GeometryConfig &config, bool autoRelease) = 0;
};