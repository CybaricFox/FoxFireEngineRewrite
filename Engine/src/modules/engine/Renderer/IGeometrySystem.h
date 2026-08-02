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
    unsigned int vertexCount = 0;
    Vertex3d* vertices = nullptr;
    unsigned int indexCount = 0;
    unsigned int* indices = nullptr;
    String name{};
    String materialName{};
    String materialPath{};
};

class FOXFIRE_API IGeometrySystem {
public:
    virtual ~IGeometrySystem() = default;

    virtual bool initialize(unsigned int initialCapacity, RendererBackend* backend, IMaterialSystem* materialSystem) = 0;
    virtual void shutdown() = 0;

    virtual Geometry& getDefaultGeometry() = 0;

    virtual GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount, float xTile, float yTile, const String &name, const String &materialName) = 0;
    virtual Geometry& acquireGeometry(const GeometryConfig &config, bool autoRelease) = 0;
};