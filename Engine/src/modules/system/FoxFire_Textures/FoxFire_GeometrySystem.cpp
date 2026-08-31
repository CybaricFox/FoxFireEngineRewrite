//
// Created by cmorg on 8/2/2026.
//

#include "FoxFire_GeometrySystem.h"

Geometry & FoxFire_GeometrySystem::acquireGeometry(const unsigned int id) {
    if (id == INVALID_ID_U32 || geometries.get(id).geometry.id == INVALID_ID_U32) {
        Logger::logError("Geometry system cannot load geometry with an invalid id!");
        return default3DGeometry;
    }

    geometries.get(id).referenceCount++;
    return geometries.get(id).geometry;
}

Geometry & FoxFire_GeometrySystem::acquireGeometry(GeometryConfig &config, const bool autoRelease) {
    Geometry* geometry = nullptr;

    const unsigned int index = geometries.assign();
    GeometryContext& context = geometries.get(index);

    context.bAutoRelease = autoRelease;
    context.referenceCount = 1;
    geometry = &context.geometry;
    geometry->id = index;

    if (!createGeometry(config, *geometry)) {
        Logger::logError("Failed to create geometry!");
        return default3DGeometry;
    }

    return *geometry;
}

void FoxFire_GeometrySystem::releaseGeometry(const Geometry &geometry) {
    if (geometry.id == INVALID_ID_U32) {
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
    config.vertices.initialize<Vertex3d>(xCount * yCount * 4);
    config.indices.initialize<unsigned int>(xCount * yCount * 6);

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
            const auto v0 = reinterpret_cast<Vertex3d *>(config.vertices.getVertex(vOffset + 0));
            const auto v1 = reinterpret_cast<Vertex3d *>(config.vertices.getVertex(vOffset + 1));
            const auto v2 = reinterpret_cast<Vertex3d *>(config.vertices.getVertex(vOffset + 2));
            const auto v3 = reinterpret_cast<Vertex3d *>(config.vertices.getVertex(vOffset + 3));

            v0->position.x = minX;
            v0->position.y = minY;
            v0->textureCoordinate.x = minUVX;
            v0->textureCoordinate.y = minUVY;

            v1->position.x = maxX;
            v1->position.y = maxY;
            v1->textureCoordinate.x = maxUVX;
            v1->textureCoordinate.y = maxUVY;

            v2->position.x = minX;
            v2->position.y = maxY;
            v2->textureCoordinate.x = minUVX;
            v2->textureCoordinate.y = maxUVY;

            v3->position.x = maxX;
            v3->position.y = minY;
            v3->textureCoordinate.x = maxUVX;
            v3->textureCoordinate.y = minUVY;

            const unsigned int iOffset = ((y * xCount) + x) * 6;
            config.indices.setIndex(vOffset + 0, iOffset + 0);
            config.indices.setIndex(vOffset + 1, iOffset + 1);
            config.indices.setIndex(vOffset + 2, iOffset + 2);
            config.indices.setIndex(vOffset + 0, iOffset + 3);
            config.indices.setIndex(vOffset + 3, iOffset + 4);
            config.indices.setIndex(vOffset + 1, iOffset + 5);
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

bool FoxFire_GeometrySystem::createDefaultGeometries() {
    constexpr float f = 10.0f;

    //3d geometry
    Vertex3d vertices3D[4]{};

    vertices3D[0].position = {-0.5 * f, -0.5f * f, 0};
    vertices3D[0].textureCoordinate = {0, 0};
    vertices3D[1].position = {0.5f * f, 0.5f * f, 0};
    vertices3D[1].textureCoordinate = {1, 1};
    vertices3D[2].position = {-0.5 * f, 0.5f * f, 0};
    vertices3D[2].textureCoordinate = {0, 1};
    vertices3D[3].position = {0.5 * f, -0.5f * f, 0};
    vertices3D[3].textureCoordinate = {1, 0};

    unsigned int indices3D[6] = {0, 1, 2, 0, 3, 1};

    if (!backendRef->createGeometry(default3DGeometry, sizeof(Vertex3d), 4, vertices3D, sizeof(unsigned int),6 , indices3D)) {
        Logger::logFatal("Failed to create default 3D geometry!");
        return false;
    }

    default3DGeometry.material = &materialSystemRef->getDefaultMaterial();

    //2d geometry
    Vertex3d vertices2D[4];
    FF_Memory::ff_clear(vertices2D, sizeof(Vertex3d) * 4);

    vertices2D[0].position = {-0.5 * f, -0.5f * f, 0};
    vertices2D[0].textureCoordinate = {0, 0};
    vertices2D[1].position = {0.5f * f, 0.5f * f, 0};
    vertices2D[1].textureCoordinate = {1, 1};
    vertices2D[2].position = {-0.5 * f, 0.5f * f, 0};
    vertices2D[2].textureCoordinate = {0, 1};
    vertices2D[3].position = {0.5 * f, -0.5f * f, 0};
    vertices2D[3].textureCoordinate = {1, 0};

    unsigned int indices2D[6] = {2, 1, 0, 3, 0, 1};

    if (!backendRef->createGeometry(default2DGeometry, sizeof(Vertex2d), 4, vertices2D, sizeof(unsigned int),6 , indices2D)) {
        Logger::logFatal("Failed to create default 2D geometry!");
        return false;
    }

    default2DGeometry.material = &materialSystemRef->getDefaultMaterial();

    return true;
}

bool FoxFire_GeometrySystem::createGeometry(GeometryConfig &config, Geometry &geometry) {
    if (!backendRef->createGeometry(geometry, config.vertices.getSize(), config.vertices.getCount(), config.vertices.getVertex(0), config.indices.getSize(), config.indices.getCount(), config.indices.getIndex(0))) {
        geometries.get(geometry.id).referenceCount = 0;
        geometries.get(geometry.id).bAutoRelease = false;
        geometry.id = INVALID_ID_U32;
        geometry.generation = INVALID_ID_U32;
        geometry.internalId = INVALID_ID_U32;

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

GeometryConfig FoxFire_GeometrySystem::generateCubeConfig(float width, float height, const float depth, float xTile, float yTile, const String &name, const String &materialName) {
    if (width == 0) {
        Logger::logWarn("Plane width cannot be 0.");
        width = 1;
    }
    if (height == 0) {
        Logger::logWarn("Plane height cannot be 0.");
        height = 1;
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
    config.vertices.initialize<Vertex3d>(4 * 6);
    config.indices.initialize<unsigned int>(6 * 6);

    const float halfWidth = width * 0.5f;
    const float halfHeight = height * 0.5f;
    const float halfDepth = depth * 0.5f;
    const float minX = -halfWidth;
    const float maxX = halfWidth;
    const float minY = -halfHeight;
    const float maxY = halfHeight;
    const float minZ = -halfDepth;
    const float maxZ = halfDepth;
    const float minUVX = 0;
    const float minUVY = 0;
    const float maxUVX = xTile;
    const float maxUVY = yTile;

    Vertex3d vertices[24]{};

    vertices[(0 * 4) + 0].position = {minX, minY, maxZ};
    vertices[(0 * 4) + 1].position = {maxX, maxY, maxZ};
    vertices[(0 * 4) + 2].position = {minX, maxY, maxZ};
    vertices[(0 * 4) + 3].position = {maxX, minY, maxZ};
    vertices[(0 * 4) + 0].textureCoordinate = {minUVX, minUVY};
    vertices[(0 * 4) + 1].textureCoordinate = {maxUVX, maxUVY};
    vertices[(0 * 4) + 2].textureCoordinate = {minUVX, maxUVY};
    vertices[(0 * 4) + 3].textureCoordinate = {maxUVX, minUVY};
    vertices[(0 * 4) + 0].normal = {0, 0, 1};
    vertices[(0 * 4) + 1].normal = {0, 0, 1};
    vertices[(0 * 4) + 2].normal = {0, 0, 1};
    vertices[(0 * 4) + 3].normal = {0, 0, 1};

    vertices[(1 * 4) + 0].position = {maxX, minY, minZ};
    vertices[(1 * 4) + 1].position = {minX, maxY, minZ};
    vertices[(1 * 4) + 2].position = {maxX, maxY, minZ};
    vertices[(1 * 4) + 3].position = {minX, minY, minZ};
    vertices[(1 * 4) + 0].textureCoordinate = {minUVX, minUVY};
    vertices[(1 * 4) + 1].textureCoordinate = {maxUVX, maxUVY};
    vertices[(1 * 4) + 2].textureCoordinate = {minUVX, maxUVY};
    vertices[(1 * 4) + 3].textureCoordinate = {maxUVX, minUVY};
    vertices[(1 * 4) + 0].normal = {0, 0, -1};
    vertices[(1 * 4) + 1].normal = {0, 0, -1};
    vertices[(1 * 4) + 2].normal = {0, 0, -1};
    vertices[(1 * 4) + 3].normal = {0, 0, -1};

    vertices[(2 * 4) + 0].position = {minX, minY, minZ};
    vertices[(2 * 4) + 1].position = {minX, maxY, maxZ};
    vertices[(2 * 4) + 2].position = {minX, maxY, minZ};
    vertices[(2 * 4) + 3].position = {minX, minY, maxZ};
    vertices[(2 * 4) + 0].textureCoordinate = {minUVX, minUVY};
    vertices[(2 * 4) + 1].textureCoordinate = {maxUVX, maxUVY};
    vertices[(2 * 4) + 2].textureCoordinate = {minUVX, maxUVY};
    vertices[(2 * 4) + 3].textureCoordinate = {maxUVX, minUVY};
    vertices[(2 * 4) + 0].normal = {-1, 0, 0};
    vertices[(2 * 4) + 1].normal = {-1, 0, 0};
    vertices[(2 * 4) + 2].normal = {-1, 0, 0};
    vertices[(2 * 4) + 3].normal = {-1, 0, 0};

    vertices[(3 * 4) + 0].position = {maxX, minY, maxZ};
    vertices[(3 * 4) + 1].position = {maxX, maxY, minZ};
    vertices[(3 * 4) + 2].position = {maxX, maxY, maxZ};
    vertices[(3 * 4) + 3].position = {maxX, minY, minZ};
    vertices[(3 * 4) + 0].textureCoordinate = {minUVX, minUVY};
    vertices[(3 * 4) + 1].textureCoordinate = {maxUVX, maxUVY};
    vertices[(3 * 4) + 2].textureCoordinate = {minUVX, maxUVY};
    vertices[(3 * 4) + 3].textureCoordinate = {maxUVX, minUVY};
    vertices[(3 * 4) + 0].normal = {1, 0, 0};
    vertices[(3 * 4) + 1].normal = {1, 0, 0};
    vertices[(3 * 4) + 2].normal = {1, 0, 0};
    vertices[(3 * 4) + 3].normal = {1, 0, 0};

    vertices[(4 * 4) + 0].position = {maxX, minY, maxZ};
    vertices[(4 * 4) + 1].position = {minX, minY, minZ};
    vertices[(4 * 4) + 2].position = {maxX, minY, minZ};
    vertices[(4 * 4) + 3].position = {minX, minY, maxZ};
    vertices[(4 * 4) + 0].textureCoordinate = {minUVX, minUVY};
    vertices[(4 * 4) + 1].textureCoordinate = {maxUVX, maxUVY};
    vertices[(4 * 4) + 2].textureCoordinate = {minUVX, maxUVY};
    vertices[(4 * 4) + 3].textureCoordinate = {maxUVX, minUVY};
    vertices[(4 * 4) + 0].normal = {0, -1, 0};
    vertices[(4 * 4) + 1].normal = {0, -1, 0};
    vertices[(4 * 4) + 2].normal = {0, -1, 0};
    vertices[(4 * 4) + 3].normal = {0, -1, 0};

    vertices[(5 * 4) + 0].position = {minX, maxY, maxZ};
    vertices[(5 * 4) + 1].position = {maxX, maxY, minZ};
    vertices[(5 * 4) + 2].position = {minX, maxY, minZ};
    vertices[(5 * 4) + 3].position = {maxX, maxY, maxZ};
    vertices[(5 * 4) + 0].textureCoordinate = {minUVX, minUVY};
    vertices[(5 * 4) + 1].textureCoordinate = {maxUVX, maxUVY};
    vertices[(5 * 4) + 2].textureCoordinate = {minUVX, maxUVY};
    vertices[(5 * 4) + 3].textureCoordinate = {maxUVX, minUVY};
    vertices[(5 * 4) + 0].normal = {0, 1, 0};
    vertices[(5 * 4) + 1].normal = {0, 1, 0};
    vertices[(5 * 4) + 2].normal = {0, 1, 0};
    vertices[(5 * 4) + 3].normal = {0, 1, 0};

    for (unsigned int i = 0; i < 24; i++) {
        config.vertices.setVertex(&vertices[i], i);
    }

    for (unsigned int i = 0; i < 6; i++) {
        const unsigned int vOffset = i * 4;
        const unsigned int iOffset = i * 6;
        config.indices.setIndex(vOffset + 0, iOffset + 0);
        config.indices.setIndex(vOffset + 1, iOffset + 1);
        config.indices.setIndex(vOffset + 2, iOffset + 2);
        config.indices.setIndex(vOffset + 0, iOffset + 3);
        config.indices.setIndex(vOffset + 3, iOffset + 4);
        config.indices.setIndex(vOffset + 1, iOffset + 5);
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

FoxFire_GeometrySystem::FoxFire_GeometrySystem()
    :IGeometrySystem(sizeof(FoxFire_GeometrySystem))
{

}

FoxFire_GeometrySystem::~FoxFire_GeometrySystem() = default;

bool FoxFire_GeometrySystem::initialize(const unsigned initialCapacity, IRendererBackend *backend, IMaterialSystem *materialSystem, ResourceSystem *resources) {
    IGeometrySystem::initialize(initialCapacity, backend, materialSystem, resources);

    geometries.initialize(initialCapacity);

    if (!createDefaultGeometries()) {
        Logger::logFatal("Failed to create default geometry");
        return false;
    }

    return true;
}
