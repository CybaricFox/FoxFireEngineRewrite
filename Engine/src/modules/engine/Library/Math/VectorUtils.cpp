//
// Created by cmorg on 11/14/2026.
//

#include "VectorUtils.h"

#include "FF_Math.h"
#include "MatrixUtils.h"

bool VectorUtils::compareVectors(Vector2f a, const Vector2f b, const float tolerance) {
    a -= b;
    if (FF_Math::abs(a.x) > tolerance) {
        return false;
    }
    if (FF_Math::abs(a.y) > tolerance) {
        return false;
    }
    return true;
}

bool VectorUtils::compareVectors(Vector3f a, const Vector3f b, const float tolerance) {
    a -= b;
    if (FF_Math::abs(a.x) > tolerance) {
        return false;
    }
    if (FF_Math::abs(a.y) > tolerance) {
        return false;
    }
    if (FF_Math::abs(a.z) > tolerance) {
        return false;
    }
    return true;
}

bool VectorUtils::compareVectors(Vector4f a, const Vector4f b, const float tolerance) {
    a -= b;
    if (FF_Math::abs(a.x) > tolerance) {
        return false;
    }
    if (FF_Math::abs(a.y) > tolerance) {
        return false;
    }
    if (FF_Math::abs(a.z) > tolerance) {
        return false;
    }
    if (FF_Math::abs(a.w) > tolerance) {
        return false;
    }
    return true;
}

Vector3f VectorUtils::transformVector3(const Vector3f vector, const Mat4 &matrix) {
    Vector3f out{};
    out.x = vector.x * matrix.data[0 + 0] + vector.y * matrix.data[4 + 0] + vector.z * matrix.data[8 + 0] + 1.0f * matrix.data[12 + 0];
    out.y = vector.x * matrix.data[0 + 1] + vector.y * matrix.data[4 + 1] + vector.z * matrix.data[8 + 1] + 1.0f * matrix.data[12 + 1];
    out.z = vector.x * matrix.data[0 + 2] + vector.y * matrix.data[4 + 2] + vector.z * matrix.data[8 + 2] + 1.0f * matrix.data[12 + 2];
    return out;
}

float VectorUtils::getVectorLength(const Vector2f &vector) {
    return FF_Math::sqrt(getLengthSquared(vector));
}

float VectorUtils::getVectorLength(const Vector3f &vector) {
    return FF_Math::sqrt(getLengthSquared(vector));
}

float VectorUtils::getVectorLength(const Vector4f &vector) {
    return FF_Math::sqrt(getLengthSquared(vector));
}

Vector3f VectorUtils::getVectorCrossProduct(const Vector3f a, const Vector3f b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float VectorUtils::getVectorDotProduct(Vector4f a, Vector4f b) {
#if defined(USE_SIMD)
    __m128 mul = _mm_mul_ps(a.data, b.data);

    __m128 shuf = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(mul, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);

    return _mm_cvtss_float(sums);
#else
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
#endif
}

void VectorUtils::normalize(Vector2f *vector) {
    const float lengthSq = getLengthSquared(*vector);
    if (lengthSq <= FF_EPSILON) {
        *vector = zeroVector2f();
        return;
    }
    const float length = 1.0f / FF_Math::sqrt(lengthSq);
    vector->x *= length;
    vector->y *= length;
}

void VectorUtils::normalize(Vector3f *vector) {
    const float lengthSq = getLengthSquared(*vector);
    if (lengthSq <= FF_EPSILON) {
        *vector = zeroVector3f();
        return;
    }
    const float length = 1.0f / FF_Math::sqrt(lengthSq);
    vector->x *= length;
    vector->y *= length;
    vector->z *= length;
}

void VectorUtils::normalize(Vector4f *vector) {
    const float lengthSq = getLengthSquared(*vector);
    if (lengthSq <= FF_EPSILON) {
        *vector = zeroVector4f();
        return;
    }
    const float length = 1.0f / FF_Math::sqrt(lengthSq);
    vector->x *= length;
    vector->y *= length;
    vector->z *= length;
    vector->w *= length;
}

Vector3f VectorUtils::getBackwardDirection(const Mat4 &matrix) {
    Vector3f forward{};
    forward.x = matrix.data[2];
    forward.y = matrix.data[6];
    forward.z = matrix.data[10];
    normalize(forward);
    return forward;
}

Vector3f VectorUtils::getUpDirection(const Mat4 &matrix) {
    Vector3f up{};
    up.x = matrix.data[1];
    up.y = matrix.data[5];
    up.z = matrix.data[9];
    normalize(up);
    return up;
}

Vector3f VectorUtils::getRightDirection(const Mat4 &matrix) {
    Vector3f right{};
    right.x = matrix.data[0];
    right.y = matrix.data[4];
    right.z = matrix.data[8];
    normalize(right);
    return right;
}

Quat QuatUtils::multiplyQuat(const Quat a, const Quat b) {
    Quat out;

    out.x = a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x;
    out.y = -a.x * b.z + a.y * b.w + a.z * b.x + a.w * b.y;
    out.z = a.x * b.y - a.y * b.x + a.z * b.w + a.w * b.z;
    out.w = -a.x * b.x - a.y * b.y - a.z * b.z + a.w * b.w;

    return out;
}

float QuatUtils::getQuatNormal(const Quat q) {
    return FF_Math::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

Mat4 QuatUtils::convertQuatToMatrix(const Quat q) {
    Mat4 out = MatrixUtils::matrixIdentity();

    // https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix

    const Quat n = normalizeQuat(q);

    out.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    out.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    out.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

    out.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    out.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    out.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

    out.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    out.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    out.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

    return out;
}

Quat QuatUtils::getQuatFromAxisAngle(const Vector3f axis, const float angle, const bool shouldNormalize) {
    const float halfAngle = 0.5f * angle;
    const float s = FF_Math::sin(halfAngle);
    const float c = FF_Math::cos(halfAngle);
    const auto q = Quat{s * axis.x, s * axis.y, s * axis.z, c};
    if (shouldNormalize) {
        return normalizeQuat(q);
    }
    return q;
}

Quat QuatUtils::slerpQuat(Quat a, Quat b, float percentage) {
    // Source: https://en.wikipedia.org/wiki/Slerp
    // Only unit quaternions are valid rotations.
    // Normalize to avoid undefined behavior.
    Quat normalA = normalizeQuat(a);
    Quat normalB = normalizeQuat(b);

    // Compute the cosine of the angle between the two vectors.
    float dot = getQuatDotProduct(normalA, normalB);

    // If the dot product is negative, slerp won't take
    // the shorter path. Note that normalB and -normalB are equivalent when
    // the negation is applied to all four components. Fix by
    // reversing one quaternion.
    if (dot < 0.0f) {
        normalB.x = -normalB.x;
        normalB.y = -normalB.y;
        normalB.z = -normalB.z;
        normalB.w = -normalB.w;
        dot = -dot;
    }

    if (dot > 0.9995f) {
        // If the inputs are too close for comfort, liffNearly interpolate
        // and normalize the result.
        Quat out{
            normalA.x + ((normalB.x - normalA.x) * percentage),
            normalA.y + ((normalB.y - normalA.y) * percentage),
            normalA.z + ((normalB.z - normalA.z) * percentage),
            normalA.w + ((normalB.w - normalA.w) * percentage)};
        return normalizeQuat(out);
    }

    // Since dot is in range [0, 0.9995f], acos is safe
    float theta_0 = FF_Math::acos(dot);          // theta_0 = angle between input vectors
    float theta = theta_0 * percentage;  // theta = angle between v0 and result
    float sin_theta = FF_Math::sin(theta);       // compute this value only once
    float sin_theta_0 = FF_Math::sin(theta_0);   // compute this value only once

    float s0 = FF_Math::cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
    float s1 = sin_theta / sin_theta_0;

    return Quat{
        (normalA.x * s0) + (normalB.x * s1),
        (normalA.y * s0) + (normalB.y * s1),
        (normalA.z * s0) + (normalB.z * s1),
        (normalA.w * s0) + (normalB.w * s1)};
}
