//
// Created by cmorg on 9/12/2026.
//

#include "CameraSystem.h"

#include "src/modules/engine/ECS/MasterEntityComponentSystem.h"
#include "src/modules/engine/ECS/Engine_Components/Transform.h"

bool CameraSystem::initialize(const CameraSystemConfig systemConfig, MasterEntityComponentSystem *ecs) {
    if (systemConfig.maxCameraCount == 0) {
        Logger::logFatal("Camera system config contains a max camera count of 0!");
        return false;
    }

    ecsRef = ecs;

    //Create a camera entity and set the default camera
    Entity* cameraEntity = ecs->createEntityType("Camera_Entity");

    cameraEntity->components.initialize(0, ECS);
    const auto cameraTransform = FF_Memory::ff_allocate_class<Transform>(sizeof(Transform), ECS);
    cameraEntity->components.push(cameraTransform);
    const auto cameraComponent = FF_Memory::ff_allocate_class<Camera>(sizeof(Camera), ECS);
    cameraEntity->components.push(cameraComponent);

    //Create a default camera. This camera will be used as a fallback.
    const unsigned int camera = ecsRef->createEntity("Camera_Entity");
    defaultId = camera;

    return true;
}

void CameraSystem::shutdown() {
    ecsRef = nullptr;
}

Camera * CameraSystem::aquireCamera(const unsigned int id) const {
    if (id == defaultId) {
        auto camera = MasterEntityComponentSystem::getComponent<Camera>(id);
        camera->referenceCount++;
        return camera;
    }

    const auto camera = MasterEntityComponentSystem::getComponent<Camera>(id);
    if (!camera) {
        MasterEntityComponentSystem::addComponent<Camera>(id);
        return nullptr;
    }

    camera->referenceCount++;
    return camera;
}

void CameraSystem::releaseCamera(const unsigned int id) {
    const auto camera = MasterEntityComponentSystem::getComponent<Camera>(id);
    camera->referenceCount--;

    if (camera->referenceCount == 0 && camera->bAutoRelease) {
        MasterEntityComponentSystem::removeComponent<Camera>(id);
    }
}
