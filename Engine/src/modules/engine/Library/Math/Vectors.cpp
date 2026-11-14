//
// Created by cmorg on 11/14/2026.
//

#include "Vectors.h"

#include "VectorUtils.h"

bool operator==(const Vector2f &left, const Vector2f &right) {
    return VectorUtils::compareVectors(left, right, FF_EPSILON);
}

bool operator==(const Vector3f &left, const Vector3f &right) {
    return VectorUtils::compareVectors(left, right, FF_EPSILON);
}

bool operator==(const Vector4f &left, const Vector4f &right) {
    return VectorUtils::compareVectors(left, right, FF_EPSILON);
}
