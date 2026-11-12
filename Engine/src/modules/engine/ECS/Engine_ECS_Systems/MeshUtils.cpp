//
// Created by cmorg on 10/2/2026.
//

#include "MeshUtils.h"

#include "src/modules/engine/ECS/EntityComponentSystem.h"
#include "src/modules/engine/Library/GeometryUtils.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

InstanceTracker<Mesh> MeshUtils::meshIdGenerator{};

unsigned int MeshUtils::loadMeshSuccess(void *params) {
    MeshParams& meshParams = *static_cast<MeshParams *>(params);
    const auto geometryConfigs = static_cast<DynamicArray<GeometryConfig>*>(meshParams.meshResource.data);

    Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(meshParams.meshEntity);

    mesh.geometryCount = meshParams.meshResource.dataSize;
    mesh.geometries.initialize(mesh.geometryCount);
    for (unsigned int i = 0; i < mesh.geometryCount; i++) {
        mesh.geometries.push(&GeometrySystem::acquireGeometry((*geometryConfigs)[i], true));
    }

    mesh.generation++;

    Logger::logDebug("Successfully loaded mesh " + meshParams.name);

    ResourceSystem::unload(meshParams.meshResource);

    return 0;
}

unsigned int MeshUtils::loadMeshFailed(void *params) {
    MeshParams& meshParams = *static_cast<MeshParams *>(params);
    Logger::logError("Failed to load mesh " + meshParams.name);
    ResourceSystem::unload(meshParams.meshResource);
    return 0;
}

bool MeshUtils::loadMeshStart(void *params, void *result) {
    MeshParams& meshParams = *static_cast<MeshParams *>(params);

    const bool loadResult = ResourceSystem::load(meshParams.name, RESOURCE_TYPE_MESH, meshParams.meshResource, nullptr);

    meshParams.copyTo(static_cast<IThreadParam *>(result));

    return loadResult;
}

void MeshUtils::shutdownIdTracker() {
    meshIdGenerator.shutdown();
}

bool MeshUtils::loadMeshFromResource(const String &name, const unsigned int meshEntity) {
    Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(meshEntity);
    mesh.generation = INVALID_ID_U8;

    MeshParams params{};
    params.name = name;
    params.meshEntity = meshEntity;
    params.meshResource = Resource{};

    const JobContext context = JobSystem::getInstance().createJob<MeshParams, MeshParams>(loadMeshStart, loadMeshSuccess, loadMeshFailed, &params);
    JobSystem::getInstance().submit(context);

    return true;
}

void MeshUtils::unloadMesh(const unsigned int meshEntity) {
    Mesh* mesh = EntityComponentSystem::getComponent<Mesh>(meshEntity);
    if (!mesh) return;

    for (unsigned int i = 0; i < mesh->geometryCount; i++) {
        GeometrySystem::releaseGeometry(*mesh->geometries[i]);
    }

    mesh->geometries.shutdown();
    mesh->generation = INVALID_ID_U8;
}

unsigned int MeshUtils::generateMeshId(Mesh *mesh) {
    return meshIdGenerator.acquireId(mesh);
}

void MeshUtils::moveMeshLocation(const unsigned int id, Mesh *mesh) {
    meshIdGenerator.overrideInstance(id, mesh);
}

void MeshUtils::freeMesh(const unsigned int id, const Mesh* mesh) {
    if (mesh == nullptr || mesh != meshIdGenerator.getInstance(id)) return;

    meshIdGenerator.freeId(id);
}
