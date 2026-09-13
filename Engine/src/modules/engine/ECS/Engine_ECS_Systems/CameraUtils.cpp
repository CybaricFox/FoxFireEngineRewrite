//
// Created by cmorg on 9/12/2026.
//

#include "CameraUtils.h"

#include <algorithm>

Mat4 CameraUtils::getViewMatrix(Camera &camera) {
    if (camera.bIsDirty) {
        const Mat4 rotation = createEuler(camera.rotation);
        const Mat4 translation = createTranslationMatrix(camera.position);

        camera.viewMatrix = rotation * translation;
        camera.viewMatrix = invertMatrix(camera.viewMatrix);
        camera.bIsDirty = false;
    }

    return camera.viewMatrix;
}

void CameraUtils::adjustYaw(Camera &camera, const float amount) {
    camera.rotation.y += amount;
    camera.bIsDirty = true;
}

void CameraUtils::adjustPitch(Camera &camera, const float amount) {
    camera.rotation.x += amount;
    static constexpr float limit = 1.55334306f; //= 89 degrees
    camera.rotation.x = std::clamp(camera.rotation.x, -limit, limit);
    camera.bIsDirty = true;
}

void CameraUtils::resetCamera(Camera &camera) {
    camera.rotation = zeroVector3f();
    camera.position = zeroVector3f();
    camera.bIsDirty = false;
    camera.viewMatrix = matrixIdentity();
}

void CameraUtils::moveForward(Camera &camera, const float amount) {
    Vector3f direction = getForward(camera);
    direction *= amount;
    camera.position += direction;
    camera.bIsDirty = true;
}

void CameraUtils::moveBackward(Camera &camera, const float amount) {
    Vector3f direction = getBackward(camera);
    direction *= amount;
    camera.position += direction;
    camera.bIsDirty = true;
}

void CameraUtils::moveRight(Camera &camera, const float amount) {
    Vector3f direction = getRight(camera);
    direction *= amount;
    camera.position += direction;
    camera.bIsDirty = true;
}

void CameraUtils::moveLeft(Camera &camera, const float amount) {
    Vector3f direction = getLeft(camera);
    direction *= amount;
    camera.position += direction;
    camera.bIsDirty = true;
}

void CameraUtils::moveUp(Camera &camera, const float amount) {
    Vector3f direction = upVector3f();
    direction *= amount;
    camera.position += direction;
    camera.bIsDirty = true;
}

void CameraUtils::moveDown(Camera &camera, const float amount) {
    Vector3f direction = downVector3f();
    direction *= amount;
    camera.position += direction;
    camera.bIsDirty = true;
}
