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
 *  @brief Collection of camera related utility functions.
 *  @version 1.0
 *  @date 9/12/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API CameraUtils {
private:
    /**
     * @brief Gets the forward direction of the camera
     * @param camera
     * @return
     */
    static Vector3f getForward(Camera& camera) {return getForwardDirection(getViewMatrix(camera));}
    /**
     * @brief Gets the backward direction of the camera
     * @param camera
     * @return
     */
    static Vector3f getBackward(Camera& camera) {return getBackwardDirection(getViewMatrix(camera));}
    /**
     * @brief Gets the right direction of the camera
     * @param camera
     * @return
     */
    static Vector3f getRight(Camera& camera) {return getRightDirection(getViewMatrix(camera));}
    /**
     * @brief Gets the left direction of the camera
     * @param camera
     * @return
     */
    static Vector3f getLeft(Camera& camera){return getLeftDirection(getViewMatrix(camera));}
    /**
     * @brief Gets the upward direction of the camera
     * @param camera
     * @return
     */
    static Vector3f getUp(Camera& camera) {return getUpDirection(getViewMatrix(camera));}
    /**
     * @brief Gets the downward direction of the camera
     * @param camera
     * @return
     */
    static Vector3f getDown(Camera& camera) {return getDownDirection(getViewMatrix(camera));}

public:
    /**
     * @brief Gets the camera position.
     * @param camera
     * @return
     */
    static Vector3f getPosition(const Camera& camera) {return camera.position;}
    /**
     * @brief Gets the camera rotation.
     * @param camera
     * @return
     */
    static Vector3f getRotation(const Camera& camera) {return camera.rotation;}
    /**
     * @brief Gets a copy of the camera's view matrix.
     * @param camera
     * @return
     */
    static Mat4 getViewMatrix(Camera &camera);

    /**
     * @brief Sets the camera position
     * @param camera
     * @param position Postion to set to.
     */
    static void setPosition(Camera& camera, const Vector3f position) {
        camera.position = position;
        camera.bIsDirty = true;
    }

    /**
     * @brief Sets the camera rotation.
     * @param camera
     * @param rotation Rotation to set to.
     */
    static void setRotation(Camera& camera, const Vector3f rotation) {
        camera.rotation = rotation;
        camera.bIsDirty = true;
    }

    /**
     * @brief Sets the camera data to default values.
     * @param camera
     */
    static void resetCamera(Camera& camera);

    /**
     * @brief Moves the camera forward
     * @param camera
     * @param amount Amount to move
     */
    static void moveForward(Camera& camera, float amount);
    /**
     * @brief Moves the camera backward
     * @param camera
     * @param amount Amount to move
     */
    static void moveBackward(Camera& camera, float amount);
    /**
     * @brief Moves the camera right
     * @param camera
     * @param amount Amount to move
     */
    static void moveRight(Camera& camera, float amount);
    /**
     * @brief Moves the camera left
     * @param camera
     * @param amount Amount to move
     */
    static void moveLeft(Camera& camera, float amount);
    /**
     * @brief Moves the camera up
     * @param camera
     * @param amount Amount to move
     */
    static void moveUp(Camera& camera, float amount);
    /**
     * @brief Moves the camera down
     * @param camera
     * @param amount Amount to move
     */
    static void moveDown(Camera& camera, float amount);
    /**
         * @brief Rotates the camera on the yaw axis
         * @param camera
         * @param amount Amount to move
         */
    static void adjustYaw(Camera& camera, float amount);
    /**
     * @brief Rotates the camera on the pitch axis.
     * @param camera
     * @param amount Amount to move
     */
    static void adjustPitch(Camera& camera, float amount);
    //No roll to prevent gimble lock
};