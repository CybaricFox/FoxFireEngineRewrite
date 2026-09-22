//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "src/modules/engine/ECS/ECSTypes.h"
#include "src/modules/engine/Library/FF_Math.h"

struct Camera final : EntityComponentWrapper<Camera>{
    Vector3f position{};
    Vector3f rotation{};
    bool bIsDirty = false;
    Mat4 viewMatrix{};
    unsigned short referenceCount = 0;
    bool bAutoRelease = false;
};
