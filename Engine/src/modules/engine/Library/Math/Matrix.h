//
// Created by cmorg on 11/14/2026.
//

#pragma once

#include "foxfire_export.h"

union FOXFIRE_API Mat4 {
    alignas(16) float data[16];
#if defined(USE_SIMD)
    Vector4f rows[4];
#endif
};
inline Mat4 operator*(const Mat4& left, const Mat4& right) {
    Mat4 out{};

#if defined(USE_SIMD)
    for (unsigned int i = 0; i < 4; i++) {
        out.rows[i] =
            right.rows[0] * left.rows[i].x +
            right.rows[1] * left.rows[i].y +
            right.rows[2] * left.rows[i].z +
            right.rows[3] * left.rows[i].w;
    }
#else
    const float* a = left.data;
    const float* b = right.data;
    float* o = out.data;

    for (unsigned int i = 0; i < 4; i++) {
        for (unsigned int j = 0; j < 4; j++) {
            *o =
                a[0] * b[0  + j] +
                a[1] * b[4  + j] +
                a[2] * b[8  + j] +
                a[3] * b[12 + j];
            o++;
        }

        a += 4;
    }
#endif

    return out;
}