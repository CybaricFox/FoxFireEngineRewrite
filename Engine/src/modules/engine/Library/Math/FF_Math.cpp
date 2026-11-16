//
// Created by cmorg on 7/4/2026.
//

#include "FF_Math.h"

#include <cmath>

#include "src/modules/engine/Core/Platform.h"

bool FF_Math::bIsSeeded = false;

Frustum::Frustum(Vector3f position, Vector3f &forward, const Vector3f &right, const Vector3f &up, const float aspect, const float fov, const float near, const float far) {
    const float halfV = far * FF_Math::tan(fov * 0.5f);
    const float halfH = halfV * aspect;
    const Vector3f forwardFar = forward * far;

    sides[0] = {(forward * near) + position, forward};
    sides[1] = {position + forwardFar, forward * -1.0f};
    sides[2] = {position, VectorUtils::getVectorCrossProduct(up, forwardFar + (right * halfH))};
    sides[3] = {position, VectorUtils::getVectorCrossProduct(forwardFar - (right * halfH), up)};
    sides[4] = {position, VectorUtils::getVectorCrossProduct(right, forwardFar - (up * halfV))};
    sides[5] = {position, VectorUtils::getVectorCrossProduct(forwardFar + (up * halfV), right)};
}

void FF_Math::setSeed() {
    srand(static_cast<unsigned int>(Platform::getAbsoluteTime()));
    bIsSeeded = true;
}

float FF_Math::sin(const float value) {
    return sinf(value);
}

float FF_Math::cos(const float value) {
    return cosf(value);
}

float FF_Math::acos(const float value) {
    return acosf(value);
}

float FF_Math::tan(const float value) {
    return tanf(value);
}

float FF_Math::sqrt(const float value) {
    return sqrtf(value);
}

float FF_Math::abs(const float value) {
    return fabsf(value);
}

int FF_Math::randomInt() {
    if (!bIsSeeded) {
        setSeed();
    }
    return rand();
}

float FF_Math::randomFloat() {
    return static_cast<float>(randomInt()) / RAND_MAX;
}

int FF_Math::randomRange(const int min, const int max) {
    if (!bIsSeeded) {
        setSeed();
    }
    return rand() % (max - min + 1) + min;
}

float FF_Math::randomRange(const float min, const float max) {
    return min + static_cast<float>(randomInt()) / (RAND_MAX / (max - min));
}

float FF_Math::planeDistanceSigned(const Plane3D &plane, const Vector3f &position) {
    return VectorUtils::getVectorDotProduct(plane.normal, position) - plane.distance;
}

bool FF_Math::frustumIntersectsSphere(const Frustum &frustum, const Vector3f &center, const float radius) {
    for (auto& side : frustum.sides) {
        if (!planeIntersectsSphere(side, center, radius)) {
            return false;
        }
    }

    return true;
}

bool FF_Math::planeIntersectsSquare(const Plane3D &plane, const Vector3f &position, const Vector3f &extents) {
    const float r = extents.x * abs(plane.normal.x) + extents.y * abs(plane.normal.y) + extents.z * abs(plane.normal.z);

    return -r <= planeDistanceSigned(plane, position);
}

bool FF_Math::frustumIntersectsSquare(const Frustum &plane, const Vector3f &position, const Vector3f &extents) {
    for (auto& side : plane.sides) {
        if (!planeIntersectsSquare(side, position, extents)) {
            return false;
        }
    }

    return  true;
}
