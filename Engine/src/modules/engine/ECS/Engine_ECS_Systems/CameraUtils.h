//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "src/modules/engine/ECS/Engine_Components/Camera.h"
#include "foxfire_export.h"

/**
 *  @file CameraUtils.h
 *  @layer Engine
 *  @module ECS
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/12/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API CameraUtils {
private:
    static Vector3f getForward(Camera& camera) {return getForwardDirection(getViewMatrix(camera));}
    static Vector3f getBackward(Camera& camera) {return getBackwardDirection(getViewMatrix(camera));}
    static Vector3f getRight(Camera& camera) {return getRightDirection(getViewMatrix(camera));}
    static Vector3f getLeft(Camera& camera){return getLeftDirection(getViewMatrix(camera));}
    static Vector3f getUp(Camera& camera) {return getUpDirection(getViewMatrix(camera));}
    static Vector3f getDown(Camera& camera) {return getDownDirection(getViewMatrix(camera));}

public:
    static Vector3f getPosition(const Camera& camera) {return camera.position;}
    static Vector3f getRotation(const Camera& camera) {return camera.rotation;}
    static Mat4 getViewMatrix(Camera &camera);

    static void setPosition(Camera& camera, const Vector3f position) {
        camera.position = position;
        camera.bIsDirty = true;
    }
    static void setRotation(Camera& camera, const Vector3f rotation) {
        camera.rotation = rotation;
        camera.bIsDirty = true;
    }

    static void resetCamera(Camera& camera);

    static void moveForward(Camera& camera, float amount);
    static void moveBackward(Camera& camera, float amount);
    static void moveRight(Camera& camera, float amount);
    static void moveLeft(Camera& camera, float amount);
    static void moveUp(Camera& camera, float amount);
    static void moveDown(Camera& camera, float amount);

    static void adjustYaw(Camera& camera, float amount);
    static void adjustPitch(Camera& camera, float amount);
    //No roll to prevent gimble lock
};