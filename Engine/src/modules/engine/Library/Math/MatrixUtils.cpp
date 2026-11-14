//
// Created by cmorg on 11/14/2026.
//

#include "MatrixUtils.h"

#include "FF_Math.h"

Mat4 MatrixUtils::matrixIdentity() {
    Mat4 out{};
    out.data[0] = 1;
    out.data[5] = 1;
    out.data[10] = 1;
    out.data[15] = 1;
    return out;
}

Mat4 MatrixUtils::orthographic(const float left, const float right, const float bottom, const float top, const float ffNear, const float ffFar) {
    Mat4 out{};
    const float lr = 1.0f / (left - right);
    const float bt = 1.0f / (bottom - top);
    const float nf = 1.0f / (ffNear - ffFar);

    out.data[0] = -2.0f * lr;
    out.data[5] = -2.0f * bt;
    out.data[10] = -2.0f * nf;
    out.data[12] = (left + right) * lr;
    out.data[13] = (top + bottom) * bt;
    out.data[14] = (ffFar + ffNear) * nf;
    out.data[15] = 1.0f;
    return out;
}

Mat4 MatrixUtils::perspective(const float fovRadians, const float aspectRatio, const float ffNear, const float ffFar) {
    const float halfTanFov = FF_Math::tan(fovRadians * 0.5f);
    Mat4 out{};
    out.data[0] = 1.0f / (aspectRatio * halfTanFov);
    out.data[5] = 1.0f / halfTanFov;
    out.data[10] = ffFar / (ffNear - ffFar);
    out.data[11] = -1.0f;
    out.data[14] = (ffFar * ffNear) / (ffNear - ffFar);
    return out;
}

Mat4 MatrixUtils::lookAt(Vector3f pos, Vector3f target, Vector3f up) {
    Mat4 out{};
    Vector3f zAxis = target - pos;
    VectorUtils::normalize(zAxis);
    Vector3f xAxis = VectorUtils::getVectorCrossProduct(zAxis, up);

    if (VectorUtils::getLengthSquared(xAxis) <= FF_EPSILON) {
        up = FF_Math::abs(zAxis.y) > 0.999f ? rightVector3f() : upVector3f();
        xAxis = VectorUtils::getVectorCrossProduct(zAxis, up);
    }

    VectorUtils::normalize(xAxis);
    Vector3f yAxis = VectorUtils::getVectorCrossProduct(xAxis, zAxis);

    out.data[0] = xAxis.x;
    out.data[1] = yAxis.x;
    out.data[2] = -zAxis.x;
    out.data[3] = 0.0f;
    out.data[4] = xAxis.y;
    out.data[5] = yAxis.y;
    out.data[6] = -zAxis.y;
    out.data[7] = 0.0f;
    out.data[8] = xAxis.z;
    out.data[9] = yAxis.z;
    out.data[10] = -zAxis.z;
    out.data[11] = 0.0f;
    out.data[12] = -VectorUtils::getVectorDotProduct(xAxis, pos);
    out.data[13] = -VectorUtils::getVectorDotProduct(yAxis, pos);
    out.data[14] = VectorUtils::getVectorDotProduct(zAxis, pos);
    out.data[15] = 1.0f;
    return out;
}

Mat4 MatrixUtils::invertMatrix(const Mat4 &matrix) {
    const float* m = matrix.data;

    const float t0 = m[10] * m[15];
    const float t1 = m[14] * m[11];
    const float t2 = m[6] * m[15];
    const float t3 = m[14] * m[7];
    const float t4 = m[6] * m[11];
    const float t5 = m[10] * m[7];
    const float t6 = m[2] * m[15];
    const float t7 = m[14] * m[3];
    const float t8 = m[2] * m[11];
    const float t9 = m[10] * m[3];
    const float t10 = m[2] * m[7];
    const float t11 = m[6] * m[3];
    const float t12 = m[8] * m[13];
    const float t13 = m[12] * m[9];
    const float t14 = m[4] * m[13];
    const float t15 = m[12] * m[5];
    const float t16 = m[4] * m[9];
    const float t17 = m[8] * m[5];
    const float t18 = m[0] * m[13];
    const float t19 = m[12] * m[1];
    const float t20 = m[0] * m[9];
    const float t21 = m[8] * m[1];
    const float t22 = m[0] * m[5];
    const float t23 = m[4] * m[1];

    Mat4 out{};
    float* o = out.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    float d = m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3];

    if (FF_Math::abs(d) <= FF_EPSILON) {
        return matrixIdentity(); // Prevents invalid values, MAY NEED TO RETURN SOMETHING ELSE.
    }

    d = 1.0f / d;

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return out;
}

Mat4 MatrixUtils::createTranslationMatrix(const Vector3f &pos) {
    Mat4 out = matrixIdentity();
    out.data[12] = pos.x;
    out.data[13] = pos.y;
    out.data[14] = pos.z;
    return out;
}

Mat4 MatrixUtils::createScaleMatrix(const Vector3f &scale) {
    Mat4 out = matrixIdentity();
    out.data[0] = scale.x;
    out.data[5] = scale.y;
    out.data[10] = scale.z;
    return out;
}

Mat4 MatrixUtils::createEulerXMatrix(const float radians) {
    Mat4 out = matrixIdentity();
    const float c = FF_Math::cos(radians);
    const float s = FF_Math::sin(radians);

    out.data[5] = c;
    out.data[6] = s;
    out.data[9] = -s;
    out.data[10] = c;
    return out;
}

Mat4 MatrixUtils::createEulerYMatrix(const float radians) {
    Mat4 out = matrixIdentity();
    const float c = FF_Math::cos(radians);
    const float s = FF_Math::sin(radians);

    out.data[0] = c;
    out.data[2] = -s;
    out.data[8] = s;
    out.data[10] = c;
    return out;
}

Mat4 MatrixUtils::createEulerZMatrix(const float radians) {
    Mat4 out = matrixIdentity();
    const float c = FF_Math::cos(radians);
    const float s = FF_Math::sin(radians);

    out.data[0] = c;
    out.data[1] = s;
    out.data[4] = -s;
    out.data[5] = c;
    return out;
}

Mat4 MatrixUtils::createEuler(const float x, const float y, const float z) {
    const Mat4 rx = createEulerXMatrix(x);
    const Mat4 ry = createEulerYMatrix(y);
    const Mat4 rz = createEulerZMatrix(z);
    const Mat4 out = rx * ry * rz;
    return out;
}

Mat4 MatrixUtils::createEuler(const Vector3f rotation) {
    const Mat4 rx = createEulerXMatrix(rotation.x);
    const Mat4 ry = createEulerYMatrix(rotation.y);
    const Mat4 rz = createEulerZMatrix(rotation.z);
    const Mat4 out = rx * ry * rz;
    return out;
}

Mat4 MatrixUtils::transposeMatrix(const Mat4 &matrix) {
    Mat4 out{};
    out.data[0] = matrix.data[0];
    out.data[1] = matrix.data[4];
    out.data[2] = matrix.data[8];
    out.data[3] = matrix.data[12];
    out.data[4] = matrix.data[1];
    out.data[5] = matrix.data[5];
    out.data[6] = matrix.data[9];
    out.data[7] = matrix.data[13];
    out.data[8] = matrix.data[2];
    out.data[9] = matrix.data[6];
    out.data[10] = matrix.data[10];
    out.data[11] = matrix.data[14];
    out.data[12] = matrix.data[3];
    out.data[13] = matrix.data[7];
    out.data[14] = matrix.data[11];
    out.data[3] = matrix.data[15];
    return out;
}

Mat4 MatrixUtils::convertQuatToRotationMatrix(const Quat q, const Vector3f center) {
    Mat4 out{};

    float* o = out.data;
    o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
    o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
    o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

    o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
    o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
    o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

    o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
    o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
    o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;

    return out;
}
