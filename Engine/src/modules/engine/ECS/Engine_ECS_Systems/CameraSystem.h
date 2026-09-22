//
// Created by cmorg on 9/12/2026.
//

#pragma once

/**
 *  @file CameraSystem.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/12/2026
 *
 *  @copyright (c) 2026
 */

#define DEFAULT_CAMERA_NAME "default"
#include "src/modules/engine/ECS/MasterEntityComponentSystem.h"
#include "src/modules/engine/ECS/Engine_Components/Camera.h"

struct CameraSystemConfig {
    unsigned short maxCameraCount = 0;
};

/**
 * @brief Creates and destroys cameras.
 */
class CameraSystem {
private:
    CameraSystemConfig config{};
    unsigned int defaultId = INVALID_ID_U32;
    MasterEntityComponentSystem* ecsRef = nullptr;

public:
    bool initialize(CameraSystemConfig systemConfig, MasterEntityComponentSystem* ecs);
    void shutdown();

    [[nodiscard]] unsigned int getDefaultCamera() const {return defaultId;}

    /**
     * @brief Gets a camera from an entity or adds one if it doesn't have it. Also increments number of references to the camera.
     * @param id id of the instance
     * @return Camera component of the instance.
     */
    [[nodiscard]] Camera* aquireCamera(unsigned int id) const;

    /**
     * @brief Decrements the reference count of a camera. If it hits 0, it will be removed.
     * @param id id of the instance.
     */
    void releaseCamera(unsigned int id);
};