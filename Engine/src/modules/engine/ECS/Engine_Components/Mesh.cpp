//
// Created by cmorg on 11/9/2026.
//

#include "Mesh.h"

#include "src/modules/engine/ECS/Engine_ECS_Systems/MeshUtils.h"

Mesh::Mesh() {
    id = MeshUtils::generateMeshId(this);
}

Mesh::~Mesh() {
    geometries.shutdown();
    MeshUtils::freeMesh(id, this);
}

Mesh::Mesh(const Mesh &other)
    : geometryCount(other.geometryCount), generation(other.generation) {
        if (other.id == INVALID_ID_U32) {
            id = MeshUtils::generateMeshId(this);
        } else {
            id = other.id;
            MeshUtils::moveMeshLocation(id, this);
        }

        if (!other.geometries.isEmpty()) {
            geometries.initialize(other.geometries.getLength());

            for (Geometry* geometry : other.geometries) {
                geometries.push(geometry);
            }
        }
}




