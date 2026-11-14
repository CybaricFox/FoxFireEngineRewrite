//
// Created by cmorg on 11/14/2026.
//

#pragma once

#include "foxfire_export.h"
#include "MathGlobals.h"
#include "Matrix.h"

union Vector4f;

//VECTOR 2
union FOXFIRE_API Vector2f {
    float elements[2];

    struct {
        union {
            float x;
            float r;
            float s;
            float u;
        };
        union {
            float y;
            float g;
            float t;
            float v;
        };
    };
};
inline Vector2f& operator+=(Vector2f& source,const Vector2f& other) {
    source.x += other.x;
    source.y += other.y;
    return source;
}
inline Vector2f operator+(Vector2f left, const Vector2f& right) {
    left += right;
    return left;
}
inline Vector2f& operator-=(Vector2f& source,const Vector2f& other) {
    source.x -= other.x;
    source.y -= other.y;
    return source;
}
inline Vector2f operator-(Vector2f left, const Vector2f& right) {
    left -= right;
    return left;
}
inline Vector2f& operator*=(Vector2f& source,const Vector2f& other) {
    source.x *= other.x;
    source.y *= other.y;
    return source;
}
inline Vector2f operator*(Vector2f left, const Vector2f& right) {
    left *= right;
    return left;
}
inline Vector2f& operator/=(Vector2f& source,const Vector2f& other) {
    source.x /= other.x;
    source.y /= other.y;
    return source;
}
inline Vector2f operator/(Vector2f left, const Vector2f& right) {
    left /= right;
    return left;
}
inline bool operator==(const Vector2f& left, const Vector2f& right);

/**
 * @brief Creates a vector2 with values set to 0.
 * @return
 */
inline Vector2f zeroVector2f() {return Vector2f{0, 0};}
/**
 * @brief Creates a vector2 with values set to 1.
 * @return
 */
inline Vector2f oneVector2f() {return Vector2f{1, 1};}
/**
 * @brief Creates a vector2 with y set to 1.
 * @return
 */
inline Vector2f upVector2f() {return Vector2f{0, 1};}
/**
 * @brief Creates a vector2 with y set to -1.
 * @return
 */
inline Vector2f downVector2f() {return Vector2f{0, -1};}
/**
 * @brief Creates a vector2 with x set to -1.
 * @return
 */
inline Vector2f leftVector2f() {return Vector2f{-1, 0};}
/**
 * @brief Creates a vector2 with x set to 1.
 * @return
 */
inline Vector2f rightVector2f() {return Vector2f{1, 0};}


//VECTOR 3
union FOXFIRE_API Vector3f {
    float elements[3];

    struct {
        union {
            float x;
            float r;
            float s;
            float u;
        };
        union {
            float y;
            float g;
            float t;
            float v;
        };
        union {
            float z;
            float b;
            float p;
            float w;
        };
    };
};
inline Vector3f& operator+=(Vector3f& source,const Vector3f& other) {
    source.x += other.x;
    source.y += other.y;
    source.z += other.z;
    return source;
}
inline Vector3f operator+(Vector3f left, const Vector3f& right) {
    left += right;
    return left;
}
inline Vector3f& operator-=(Vector3f& source,const Vector3f& other) {
    source.x -= other.x;
    source.y -= other.y;
    source.z -= other.z;
    return source;
}
inline Vector3f operator-(Vector3f left, const Vector3f& right) {
    left -= right;
    return left;
}
inline Vector3f& operator*=(Vector3f& source,const Vector3f& other) {
    source.x *= other.x;
    source.y *= other.y;
    source.z *= other.z;
    return source;
}
inline Vector3f operator*(Vector3f left, const Vector3f& right) {
    left *= right;
    return left;
}
inline Vector3f& operator*=(Vector3f& source,const float scalar) {
    source.x *= scalar;
    source.y *= scalar;
    source.z *= scalar;
    return source;
}
inline Vector3f operator*(Vector3f left, const float scalar) {
    left *= scalar;
    return left;
}
inline Vector3f& operator/=(Vector3f& source,const Vector3f& other) {
    source.x /= other.x;
    source.y /= other.y;
    source.z /= other.z;
    return source;
}
inline Vector3f operator/(Vector3f left, const Vector3f& right) {
    left /= right;
    return left;
}
inline bool operator==(const Vector3f& left, const Vector3f& right);
inline Vector3f operator*(const Mat4& left, const Vector3f& right) {
    return {
        right.x * left.data[0] + right.y * left.data[1] + right.z * left.data[2] + left.data[3],
        right.x * left.data[4] + right.y * left.data[5] + right.z * left.data[6] + left.data[7],
        right.x * left.data[8] + right.y * left.data[9] + right.z * left.data[10] + left.data[11]
    };
}
inline Vector3f operator*(const Vector3f& left, const Mat4& right) {
    return {
        left.x * right.data[0] + left.y * right.data[4] + left.z * right.data[8] + right.data[12],
        left.x * right.data[1] + left.y * right.data[5] + left.z * right.data[9] + right.data[13],
        left.x * right.data[2] + left.y * right.data[6] + left.z * right.data[10] + right.data[14]
    };
}

/**
 * @brief Creates a vector3 with all values set to 0.
 * @return
 */
inline Vector3f zeroVector3f() {return Vector3f{0, 0, 0};}
/**
 * @brief Creates a vector3 with all values set to 1.
 * @return
 */
inline Vector3f oneVector3f() {return Vector3f{1, 1, 1};}
/**
 * @brief Creates a vector3 with y set to 1.
 * @return
 */
inline Vector3f upVector3f() {return Vector3f{0, 1, 0};}
/**
 * @brief Creates a vector3 with y set to -1.
 * @return
 */
inline Vector3f downVector3f() {return Vector3f{0, -1, 0};}
/**
 * @brief Creates a vector3 with x set to -1.
 * @return
 */
inline Vector3f leftVector3f() {return Vector3f{-1, 0, 0};}
/**
 * @brief Creates a vector3 with x set to 1.
 * @return
 */
inline Vector3f rightVector3f() {return Vector3f{1, 0, 0};}
/**
 * @brief Creates a vector3 with z set to -1.
 * @return
 */
inline Vector3f forwardVector3() {return Vector3f{0, 0, -1};}
/**
 * @brief Creates a vector3 with z set to 1.
 * @return
 */
inline Vector3f backwardVector3() {return Vector3f{0, 0, 1};}

//VECTOR 4
/**
 * @brief Create vector4s with createVector4f() to use SIMD.
 */
union FOXFIRE_API Vector4f {
#if defined(USE_SIMD)
    __m128 data;
#endif

    alignas(16) float elements[4];

    struct {
        union {
            float x;
            float r;
            float s;
        };
        union {
            float y;
            float g;
            float t;
        };
        union {
            float z;
            float b;
            float p;
        };
        union {
            float w;
            float a;
            float q;
        };
    };

    Vector4f() = default;
    Vector4f(const float newX, const float newY, const float newZ, const float newW) {
        #if defined(USE_SIMD)
                data = _mm_setr_ps(newX, newY, newZ, newW);
        #else
                x = newX;
                y = newY;
                z = newZ;
                w = newW;
        #endif
    }
};
inline Vector4f& operator+=(Vector4f& source,const Vector4f& other) {
#if defined(USE_SIMD)
    source.data = _mm_add_ps(source.data, other.data);
#else
    source.x += other.x;
    source.y += other.y;
    source.z += other.z;
    source.w += other.w;
#endif
    return source;
}
inline Vector4f operator+(Vector4f left, const Vector4f& right) {
    left += right;
    return left;
}
inline Vector4f& operator-=(Vector4f& source,const Vector4f& other) {
#if defined(USE_SIMD)
    source.data = _mm_sub_ps(source.data, other.data);
#else
    source.x -= other.x;
    source.y -= other.y;
    source.z -= other.z;
    source.w -= other.w;
#endif
    return source;
}
inline Vector4f operator-(Vector4f left, const Vector4f& right) {
    left -= right;
    return left;
}
inline Vector4f& operator*=(Vector4f& source,const Vector4f& other) {
#if defined(USE_SIMD)
    source.data = _mm_mul_ps(source.data, other.data);
#else
    source.x *= other.x;
    source.y *= other.y;
    source.z *= other.z;
    source.w *= other.w;
#endif
    return source;
}
inline Vector4f operator*(Vector4f left, const Vector4f& right) {
    left *= right;
    return left;
}
inline Vector4f& operator*=(Vector4f& source,const float scalar) {
#if defined(USE_SIMD)
    source.data = _mm_mul_ps(source.data, _mm_set1_ps(scalar));
#else
    source.x *= scalar;
    source.y *= scalar;
    source.z *= scalar;
    source.w *= scalar;
#endif
    return source;
}
inline Vector4f operator*(Vector4f left, const float scalar) {
    left *= scalar;
    return left;
}
inline Vector4f& operator/=(Vector4f& source,const Vector4f& other) {
#if defined(USE_SIMD)
    source.data = _mm_div_ps(source.data, other.data);
#else
    source.x /= other.x;
    source.y /= other.y;
    source.z /= other.z;
    source.w /= other.w;
#endif
    return source;
}
inline Vector4f operator/(Vector4f left, const Vector4f& right) {
    left /= right;
    return left;
}
inline bool operator==(const Vector4f& left, const Vector4f& right);
inline Vector4f operator*(const Mat4& left, const Vector4f& right) {
    return {
        right.x * left.data[0] + right.y * left.data[1] + right.z * left.data[2] + right.w * left.data[3],
        right.x * left.data[4] + right.y * left.data[5] + right.z * left.data[6] + right.w * left.data[7],
        right.x * left.data[8] + right.y * left.data[9] + right.z * left.data[10] + right.w * left.data[11],
        right.x * left.data[12] + right.y * left.data[13] + right.z * left.data[14] + right.w * left.data[15]
    };
}
inline Vector4f operator*(const Vector4f& left, const Mat4& right) {
    return {
        left.x * right.data[0] + left.y * right.data[4] + left.z * right.data[8] + left.w * right.data[12],
        left.x * right.data[1] + left.y * right.data[5] + left.z * right.data[9] + left.w * right.data[13],
        left.x * right.data[2] + left.y * right.data[6] + left.z * right.data[10] + left.w * right.data[14],
        left.x * right.data[3] + left.y * right.data[7] + left.z * right.data[11] + left.w * right.data[15]
    };
}

/**
 * @brief Creates a vector4 with all values set to 0.
 * @return
 */
inline Vector4f zeroVector4f() {return Vector4f{0, 0, 0, 0};}
/**
 * @brief Creates a vector4 with all values set to 1.
 * @return
 */
inline Vector4f oneVector4f() {return Vector4f{1, 1, 1, 1};}

//QUAT
/**
 * @brief Vector4 used for rotation.
 */
typedef Vector4f Quat;