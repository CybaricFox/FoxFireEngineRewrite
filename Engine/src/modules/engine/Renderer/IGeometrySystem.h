/**
*   @file IGeometrySystem.h
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

#include <foxfire_export.h>

#include "IMaterialSystem.h"
#include "IRendererBackend.h"
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

#define DEFAULT_GEOMETRY_NAME "default"

/**
 * @brief Holds config data for a piece of geometry
 */
struct GeometryConfig {
    /** @brief size of the vertex type */
    unsigned int vertexSize = 0;
    /** @brief number of vertices */
    unsigned int vertexCount = 0;
    /** @brief Vertex data */
    void* vertices = nullptr;
    /** @brief size of the index type */
    unsigned int indexSize = 0;
    /** @brief Number of indices */
    unsigned int indexCount = 0;
    /** @brief Index data */
    void* indices = nullptr;
    /** @brief Name of the geometry */
    String name{};
    /** @brief name of the material */
    String materialName{};
    /** @brief Path to the material */
    String materialPath{};
};

/**
 * @brief Abstract class that controls geometry.
 */
class FOXFIRE_API IGeometrySystem {
private:
    unsigned long memorySize = 0;

protected:
    IRendererBackend* backendRef = nullptr;
    IMaterialSystem* materialSystemRef = nullptr;
    ResourceSystem* resourceSystemRef = nullptr;

public:
    explicit IGeometrySystem(const unsigned long derivedSize) {memorySize = derivedSize;}
    virtual ~IGeometrySystem();

    virtual bool initialize(unsigned int initialCapacity, IRendererBackend* backend, IMaterialSystem* materialSystem, ResourceSystem* resources);

    virtual Geometry& getDefault3DGeometry() = 0;
    virtual Geometry& getDefault2DGeometry() = 0;
    [[nodiscard]] unsigned long getMemorySize() const {return memorySize;}

    virtual GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount, float xTile, float yTile, const String &name, const String &materialName) = 0;
    virtual GeometryConfig generateCubeConfig(float width, float height, float depth, float xTile, float yTile, const String &name, const String &materialName) = 0;

    /**
     * @brief Fetches geometry from where the user stores it, or creates it if it doesn't exist.
     * @param config Geometry config data
     * @param autoRelease Whether to destroy the geometry when no references remain.
     * @return The acquired geometry, or default if something goes wrong.
     */
    virtual Geometry& acquireGeometry(const GeometryConfig &config, bool autoRelease) = 0;
};