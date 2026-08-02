//
// Created by cmorg on 8/2/2026.
//

#include "FoxFire_GeometrySystem.h"

Geometry & FoxFire_GeometrySystem::acquireGeometry(const unsigned int id) {
    if (id == INVALID_ID || geometries.get(id).geometry.id == INVALID_ID) {
        Logger::logError("Geometry system cannot load geometry with an invalid id!");
        return defaultGeometry;
    }

    geometries.get(id).referenceCount++;
    return geometries.get(id).geometry;
}

Geometry & FoxFire_GeometrySystem::acquireGeometry(const GeometryConfig &config, const bool autoRelease) {
    Geometry* geometry = nullptr;

    const unsigned int index = geometries.assign();
    GeometryContext& context = geometries.get(index);

    context.bAutoRelease = autoRelease;
    context.referenceCount = 1;
    geometry = &context.geometry;
    geometry->id = index;

    if (!createGeometry(config, *geometry)) {
        Logger::logError("Failed to create geometry!");
        return defaultGeometry;
    }

    return *geometry;
}

void FoxFire_GeometrySystem::releaseGeometry(const Geometry &geometry) {
    if (geometry.id == INVALID_ID) {
        Logger::logWarn("Cannot release geometry with an invalid id!");
        return;
    }

    GeometryContext& context = geometries.get(geometry.id);
    const unsigned int id = context.geometry.id;

    if (context.geometry.id != geometry.id) {
        Logger::logFatal("Geometry ID mismatch detected!");
        return;
    }

    if (context.referenceCount > 0) {
        context.referenceCount--;
    }

    if (context.referenceCount == 0 && context.bAutoRelease) {
        destroyGeometry(context.geometry);
        context.bAutoRelease = false;
    }
}

GeometryConfig FoxFire_GeometrySystem::generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount, float xTile, float yTile, const String &name, const String &materialName) {
    if (width == 0) {
        Logger::logWarn("Plane width cannot be 0.");
        width = 1;
    }
    if (height == 0) {
        Logger::logWarn("Plane height cannot be 0.");
        height = 1;
    }
    if (xCount == 0) {
        Logger::logWarn("Plane x count cannot be 0.");
        xCount = 1;
    }
    if (yCount == 0) {
        Logger::logWarn("Plane y count cannot be 0.");
        yCount = 1;
    }
    if (xTile == 0) {
        Logger::logWarn("Plane x tile cannot be 0.");
        xTile = 1;
    }
    if (yTile == 0) {
        Logger::logWarn("Plane y tile cannot be 0.");
        yTile = 1;
    }

    GeometryConfig config{};
    config.vertexCount = xCount * yCount * 4; //4 vertices per segment
    config.vertices = static_cast<Vertex3d *>(FF_Memory::ff_allocate(sizeof(Vertex3d) * config.vertexCount, ARRAY));
    config.indexCount = xCount * yCount * 6; //6 indices per segment
    config.indices = static_cast<unsigned int *>(FF_Memory::ff_allocate(sizeof(unsigned int) * config.indexCount, ARRAY));

    const float segmentWidth = width / static_cast<float>(xCount);
    const float segmentHeight = height / static_cast<float>(yCount);
    const float halfWidth = segmentWidth * 0.5f;
    const float halfHeight = segmentHeight * 0.5f;

    for (unsigned int y = 0; y < yCount; ++y) {
        for (unsigned int x = 0; x < xCount; ++x) {
            const float minX = (static_cast<float>(x) * segmentWidth) - halfWidth;
            const float minY = (static_cast<float>(y) * segmentHeight) - halfHeight;
            const float maxX = minX + segmentWidth;
            const float maxY = minY + segmentHeight;
            const float minUVX = (static_cast<float>(x) / static_cast<float>(xCount)) * xTile;
            const float minUVY = (static_cast<float>(y) / static_cast<float>(yCount)) * yTile;
            const float maxUVX = (static_cast<float>(x + 1) / static_cast<float>(xCount)) * xTile;
            const float maxUVY = (static_cast<float>(y + 1) / static_cast<float>(yCount)) * yTile;

            const unsigned int vOffset = ((y * xCount) + x) * 4;
            Vertex3d& v0 = config.vertices[vOffset + 0];
            Vertex3d& v1 = config.vertices[vOffset + 1];
            Vertex3d& v2 = config.vertices[vOffset + 2];
            Vertex3d& v3 = config.vertices[vOffset + 3];

            v0.position.x = minX;
            v0.position.y = minY;
            v0.textureCoordinate.x = minUVX;
            v0.textureCoordinate.y = minUVY;

            v1.position.x = maxX;
            v1.position.y = maxY;
            v1.textureCoordinate.x = maxUVX;
            v1.textureCoordinate.y = maxUVY;

            v2.position.x = minX;
            v2.position.y = maxY;
            v2.textureCoordinate.x = minUVX;
            v2.textureCoordinate.y = maxUVY;

            v3.position.x = maxX;
            v3.position.y = minY;
            v3.textureCoordinate.x = maxUVX;
            v3.textureCoordinate.y = minUVY;

            const unsigned int iOffset = ((y * xCount) + x) * 6;
            config.indices[iOffset + 0] = vOffset + 0;
            config.indices[iOffset + 1] = vOffset + 1;
            config.indices[iOffset + 2] = vOffset + 2;
            config.indices[iOffset + 3] = vOffset + 0;
            config.indices[iOffset + 4] = vOffset + 3;
            config.indices[iOffset + 5] = vOffset + 1;
        }
    }

    if (!name.empty()) {
        config.name = name;
    } else {
        config.name = DEFAULT_GEOMETRY_NAME;
    }

    if (!materialName.empty()) {
        config.materialName = materialName;
    } else {
        config.materialName = DEFAULT_MATERIAL_NAME;
    }

    return config;
}

bool FoxFire_GeometrySystem::createDefaultGeometry() {
    Vertex3d vertices[4];
    FF_Memory::ff_clear(vertices, sizeof(Vertex3d) * 4);

    constexpr float f = 10.0f;

    vertices[0].position = {-0.5 * f, -0.5f * f, 0};
    vertices[0].textureCoordinate = {0, 0};
    vertices[1].position = {0.5f * f, 0.5f * f, 0};
    vertices[1].textureCoordinate = {1, 1};
    vertices[2].position = {-0.5 * f, 0.5f * f, 0};
    vertices[2].textureCoordinate = {0, 1};
    vertices[3].position = {0.5 * f, -0.5f * f, 0};
    vertices[3].textureCoordinate = {1, 0};

    constexpr unsigned int indices[6] = {0, 1, 2, 0, 3, 1};

    if (!backendRef->createGeometry(defaultGeometry, 4, vertices, 6, indices)) {
        Logger::logFatal("Failed to create default geometry!");
        return false;
    }

    defaultGeometry.material = &materialSystemRef->getDefaultMaterial();

    return true;
}

bool FoxFire_GeometrySystem::createGeometry(const GeometryConfig &config, Geometry &geometry) {
    if (!backendRef->createGeometry(geometry, config.vertexCount, config.vertices, config.indexCount, config.indices)) {
        geometries.get(geometry.id).referenceCount = 0;
        geometries.get(geometry.id).bAutoRelease = false;
        geometry.id = INVALID_ID;
        geometry.generation = INVALID_ID;
        geometry.internalId = INVALID_ID;

        return false;
    }

    if (!config.materialName.empty()) {
        geometry.material = &materialSystemRef->acquireMaterial(config.materialName);
    }

    return true;
}

void FoxFire_GeometrySystem::destroyGeometry(Geometry &geometry) {
    backendRef->destroyGeometry(geometry);

    if (geometry.material && !geometry.material->name.empty()) {
        materialSystemRef->releaseMaterial(geometry.material->name);
    }

    geometry = Geometry{};
}

FoxFire_GeometrySystem::~FoxFire_GeometrySystem() {
    shutdown();
}

void FoxFire_GeometrySystem::shutdown() {
    materialSystemRef = nullptr;
    backendRef = nullptr;
}

bool FoxFire_GeometrySystem::initialize(const unsigned initialCapacity, RendererBackend *backend, IMaterialSystem *materialSystem) {
    geometries.initialize(initialCapacity);

    backendRef = backend;
    materialSystemRef = materialSystem;

    if (!createDefaultGeometry()) {
        Logger::logFatal("Failed to create default geometry");
        return false;
    }

    return true;
}
