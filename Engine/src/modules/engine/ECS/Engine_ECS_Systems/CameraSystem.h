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

class CameraSystem {
private:
    CameraSystemConfig config{};
    unsigned int defaultId = INVALID_ID_U32;
    MasterEntityComponentSystem* ecsRef = nullptr;

public:
    bool initialize(CameraSystemConfig systemConfig, MasterEntityComponentSystem* ecs);
    void shutdown();

    [[nodiscard]] unsigned int getDefaultCamera() const {return defaultId;}

    [[nodiscard]] Camera* aquireCamera(unsigned int id) const;
    void releaseCamera(unsigned int id);
};