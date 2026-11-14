//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "src/modules/engine/ECS/ECSTypes.h"
#include "../../Library/Math/FF_Math.h"
#include "src/modules/engine/Library/Math/MatrixUtils.h"

struct Transform final : EntityComponentWrapper<Transform> {
    Vector3f position = zeroVector3f();
    Quat rotation = QuatUtils::quatIdentity();
    Vector3f scale = oneVector3f();
    bool bIsDirty = false;
    Mat4 local = MatrixUtils::matrixIdentity();
    unsigned int parent = INVALID_ID_U32;
};
