//
// Created by cmorg on 10/2/2026.
//

#pragma once
#include "foxfire_export.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/Renderer/IGeometrySystem.h"

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
    static IGeometrySystem* geometrySystemRef;
    static ResourceSystem* resourceSystemRef;

    static unsigned int loadMeshSuccess(void* params);
    static unsigned int loadMeshFailed(void* params);
    static bool loadMeshStart(void* params, void* result);
public:
    static void setGeometrySystemRef(IGeometrySystem* ref) {geometrySystemRef = ref;}
    static void setResourceSystemRef(ResourceSystem* ref) {resourceSystemRef = ref;}

    static bool loadMeshFromResource(const String &name, unsigned int meshEntity);
    static void unloadMesh(unsigned int meshEntity);
};