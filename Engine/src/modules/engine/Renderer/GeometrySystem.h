/**
*   @file GeometrySystem.h
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

#include "MaterialSystem.h"
#include "IRendererBackend.h"
#include "src/modules/engine/Library/GeometryUtils.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

#define DEFAULT_GEOMETRY_NAME "default"

/**
 * @brief Contains geometry, a count of its references, and if it should auto release.
 */
struct GeometryContext {
    Geometry geometry{};
    unsigned int referenceCount = 0;
    bool bAutoRelease = false;
};

/**
 * @brief Abstract class that controls geometry.
 */
class FOXFIRE_API GeometrySystem {
private:
    static GeometrySystem* instance;
    unsigned long memorySize = 0;
    //GeometryConfig config{};
    Geometry default3DGeometry{};
    Geometry default2DGeometry{};
    ReusableArray<GeometryContext> geometries;

    Geometry& acquireGeometry(unsigned int id);
    bool createDefaultGeometries();
    bool createGeometry(GeometryConfig &config, Geometry &geometry);
    void destroyGeometry(Geometry& geometry) const;

    IRendererBackend* backendRef = nullptr;

public:
    bool initialize(unsigned int initialCapacity, IRendererBackend *backend);
    void shutdown();

    /**
     * @brief Returns the default geometry for 3d.
     * @return
     */
    Geometry& getDefault3DGeometry() {return default3DGeometry;}

    /**
     * @brief Returns the default geometry for 2d.
     * @return
     */
    Geometry& getDefault2DGeometry() {return default2DGeometry;}

    /**
     * @brief Returns the memory size of this system.
     * @return
     */
    [[nodiscard]] unsigned long getMemorySize() const {return memorySize;}

    /**
     * @brief Generates a plane from the given argyments.
     * @param width
     * @param height
     * @param xCount
     * @param yCount
     * @param xTile
     * @param yTile
     * @param name
     * @param materialName
     * @return
     */
    static GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount, float xTile, float yTile, const String &name, const String &materialName);

    /**
     * @brief Generates a cube from the given arguments.
     * @param width
     * @param height
     * @param depth
     * @param xTile
     * @param yTile
     * @param name
     * @param materialName
     * @return
     */
    static GeometryConfig generateCubeConfig(float width, float height, float depth, float xTile, float yTile, const String &name, const String &materialName);

    /**
     * @brief Fetches geometry from where the user stores it, or creates it if it doesn't exist.
     * @param config Geometry config data
     * @param autoRelease Whether to destroy the geometry when no references remain.
     * @return The acquired geometry, or default if something goes wrong.
     */
    static Geometry& acquireGeometry(GeometryConfig &config, bool autoRelease);
    static void releaseGeometry(const Geometry& geometry);
};