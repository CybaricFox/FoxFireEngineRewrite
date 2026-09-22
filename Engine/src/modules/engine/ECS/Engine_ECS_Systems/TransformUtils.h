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
 *  @brief Collection of transform utility functions.
 *  @version 1.0
 *  @date 8/12/2026
 *
 *  @copyright (c) 2026
 */

class TransformUtils {
public:
    /**
     * @brief Converts the transform to a world position
     * @param transform Transform component
     * @return Matrix that contains the world position
     */
    static Mat4 getWorldPos(Transform& transform);

    /**
     * @brief Converts the transform to a local position
     * @param transform Transform component
     * @return Matrix that contains the local position
     */
    static Mat4 getLocalPos(Transform& transform);

    /**
     * @brief Creates a Transform component from a position
     * @param position
     * @return
     */
    static Transform createTransform(Vector3f position);
    /**
    * @brief Creates a Transform component from a rotation
    * @param rotation
    * @return
    */
    static Transform createTransform(Quat rotation);
    /**
    * @brief Creates a Transform component from a position and rotation
    * @param position
    * @param rotation
    * @return
    */
    static Transform createTransform(Vector3f position, Quat rotation);

    /**
     * @brief Creates a transform component
     * @param position Starting position
     * @param rotation Starting rotation
     * @param scale Starting scale
     * @return
     */
    static Transform createTransform(Vector3f position, Quat rotation, Vector3f scale);

    /**
     * @brief Translates the transform's position
     * @param transform
     * @param translation
     */
    static void addTranslation(Transform& transform, Vector3f translation);

    /**
     * @brief Adds a rotation to the transform
     * @param transform
     * @param rotation Rotation to add.
     */
    static void addRotation(Transform& transform, Quat rotation);

    /**
     * @brief Adds a rotation and translation to a transform
     * @param transform
     * @param translation
     * @param rotation
     */
    static void addTranslationAndRotation(Transform& transform, Vector3f translation, Quat rotation);
};