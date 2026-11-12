//
// Created by cmorg on 10/2/2026.
//

#pragma once
#include "foxfire_export.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/Library/InstanceTracker.h"
#include "src/modules/engine/Renderer/GeometrySystem.h"

/**
 *  @file MeshUtils.h
 *  @layer Engine
 *  @module ECS
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 10/2/2026
 *
 *  @copyright (c) 2026
 */

struct MeshParams final : ThreadParam<MeshParams> {
    String name{};
    unsigned int meshEntity = INVALID_ID_U32;
    Resource meshResource{};
};

class FOXFIRE_API MeshUtils {
private:
    static unsigned int loadMeshSuccess(void* params);
    static unsigned int loadMeshFailed(void* params);
    static bool loadMeshStart(void* params, void* result);

    static InstanceTracker<Mesh> meshIdGenerator;

public:
    static void shutdownIdTracker();

    static bool loadMeshFromResource(const String &name, unsigned int meshEntity);
    static void unloadMesh(unsigned int meshEntity);

    static unsigned int generateMeshId(Mesh* mesh);
    static void moveMeshLocation(unsigned int id, Mesh* mesh);
    static void freeMesh(unsigned int id, const Mesh *mesh);
};