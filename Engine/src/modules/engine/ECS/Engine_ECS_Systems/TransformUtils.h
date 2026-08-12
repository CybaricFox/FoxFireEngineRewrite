//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "src/modules/engine/ECS/Engine_Components/Transform.h"
#include "src/modules/engine/Library/FF_Math.h"

/**
 *  @file TransformUtils.h
 *  @layer Engine
 *  @module ECS
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/12/2026
 *
 *  @copyright (c) 2026
 */

class TransformUtils {
public:
    static Mat4 getWorldPos(Transform& transform);
    static Mat4 getLocalPos(Transform& transform);

    static Transform createTransform(Vector3f position);
    static Transform createTransform(Quat rotation);
    static Transform createTransform(Vector3f position, Quat rotation);
    static Transform createTransform(Vector3f position, Quat rotation, Vector3f scale);
    static void addTranslation(Transform& transform, Vector3f translation);
    static void addRotation(Transform& transform, Quat rotation);
    static void addTranslationAndRotation(Transform& transform, Vector3f translation, Quat rotation);
};