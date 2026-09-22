//
// Created by cmorg on 8/12/2026.
//

#include "TransformUtils.h"

#include "src/modules/engine/ECS/MasterEntityComponentSystem.h"

Mat4 TransformUtils::getWorldPos(Transform &transform) {
    const Mat4 local = getLocalPos(transform);

    if (transform.parent != INVALID_ID_U32) {
        const Mat4 parent = getWorldPos(*MasterEntityComponentSystem::getComponent<Transform>(transform.parent));
        return local * parent;
    }

    return local;
}

Mat4 TransformUtils::getLocalPos(Transform &transform) {
    if (transform.bIsDirty) {
        Mat4 updated = convertQuatToMatrix(transform.rotation) * createTranslationMatrix(transform.position);
        updated = createScaleMatrix(transform.scale) * updated;
        transform.local = updated;
        transform.bIsDirty = false;
    }

    return transform.local;
}

Transform TransformUtils::createTransform(const Vector3f position) {
    Transform transform{};
    transform.position = position;
    return transform;
}

Transform TransformUtils::createTransform(const Quat rotation) {
    Transform transform{};
    transform.rotation = rotation;
    return transform;
}

Transform TransformUtils::createTransform(Vector3f position, Quat rotation) {
    Transform transform{};
    transform.position = position;
    transform.rotation = rotation;
    return transform;
}

Transform TransformUtils::createTransform(Vector3f position, Quat rotation, Vector3f scale) {
    Transform transform{};
    transform.position = position;
    transform.rotation = rotation;
    transform.scale = scale;
    return transform;
}

void TransformUtils::addTranslation(Transform &transform, const Vector3f translation) {
    transform.position += translation;
    transform.bIsDirty = true;
}

void TransformUtils::addRotation(Transform &transform, const Quat rotation) {
    transform.rotation = multiplyQuat(transform.rotation, rotation);
    transform.bIsDirty = true;
}

void TransformUtils::addTranslationAndRotation(Transform &transform, const Vector3f translation, const Quat rotation) {
    transform.position += translation;
    transform.rotation = multiplyQuat(transform.rotation, rotation);
    transform.bIsDirty = true;
}
