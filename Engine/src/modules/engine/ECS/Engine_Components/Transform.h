//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "src/modules/engine/ECS/ECSTypes.h"
#include "src/modules/engine/Library/FF_Math.h"

struct Transform final : EntityComponentWrapper<Transform> {
    Vector3f position = zeroVector3f();
    Quat rotation = quatIdentity();
    Vector3f scale = oneVector3f();
    bool bIsDirty = false;
    Mat4 local = matrixIdentity();
    Transform* parent = nullptr;
};
