/**
*   @file FF_Math.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief Collection of Mathematic structures and functions.
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "foxfire_export.h"
#include "Logger.h"
#include "StringUtils.h"
#include "src/defines.h"
#include "src/modules/engine/Memory/FF_Memory.h"

#if defined(USE_SIMD)
    #include <xmmintrin.h>
#endif

/**
 * @brief PI
 */
inline constexpr float FF_PI = 3.14159265358979323846f;
/**
 * @brief A number so high it may as well be infinity
 */
inline constexpr float FF_INFINITY = 1e30f;
/**
 * @brief The lowest possible float value
 */
inline constexpr float FF_EPSILON = 1.192092869e-07f;
/**
 * @brief A multiplier used to convert degrees to radians.
 */
inline constexpr float FF_DEGREE_TO_RADIAN_MULTIPLIER = FF_PI / 180.0f;
/**
 * @brief A multiplier used to convert radians to degrees.
 */
inline constexpr float FF_RADIAN_TO_DEGREE_MULTIPLIER = 180.0f / FF_PI;

/**
 * @brief A collection of math related functions and random numbers.
 */
class FOXFIRE_API FF_Math {
private:
    /**
     * @brief Whether the random numbers from FF_Math are using a seed.
     */
    static bool bIsSeeded;

    /**
     * @brief Sets the seed to a new seed.
     */
    static void setSeed();
public:
    /**
     * @brief Applies sin to the value
     * @param value
     * @return
     */
    static float sin(float value);
    /**
     * @brief Applies cos to the value
     * @param value
     * @return
     */
    static float cos(float value);
    /**
     * @brief Applies acos to the value
     * @param value
     * @return
     */
    static float acos(float value);
    /**
     * @brief Applies tan to the value
     * @param value
     * @return
     */
    static float tan(float value);
    /**
     * @brief Square roots the value.
     * @param value
     * @return
     */
    static float sqrt(float value);
    /**
     * @brief Returns the absolute value of the value.
     * @param value
     * @return
     */
    static float abs(float value);

    /**
     * @brief Returns a random integer.
     * @return
     */
    static int randomInt();

    /**
     * @brief Returns a random float.
     * @return
     */
    static float randomFloat();

    /**
     * @brief Returns a random integer from a range.
     * @param min Minimum value.
     * @param max Maximum value.
     * @return
     */
    static int randomRange(int min, int max);
    /**
     * @brief Returns a random float from a range.
     * @param min Minimum value.
     * @param max Maximum value.
     * @return
     */
    static float randomRange(float min, float max);
};

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

/**
 * @brief Compares 2 vector2s.
 * @param a
 * @param b
 * @param tolerance How much the value can be off before its considered not equal.
 * @return True if the vectors are equal.
 */
inline bool compareVectors(Vector2f a, const Vector2f b, const float tolerance) {
    a -= b;
    if (FF_Math::abs(a.x) > tolerance) {
        return false;
    }
    if (FF_Math::abs(a.y) > tolerance) {
        return false;
    }
    return true;
}
inline bool operator==(const Vector2f& left, const Vector2f& right) {
    return compareVectors(left,right,FF_EPSILON);
}

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

/**
 * @brief Compares 2 vector3s.
 * @param a
 * @param b
 * @param tolerance How much the value can be off before its considered not equal.
 * @return True if the vectors are equal.
 */
inline bool compareVectors(Vector3f a, const Vector3f b, const float tolerance) {
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
inline bool operator==(const Vector3f& left, const Vector3f& right) {
    return compareVectors(left, right, FF_EPSILON);
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
};

/**
 * @brief Creates a vector4 with the given values.
 * @param x
 * @param y
 * @param z
 * @param w
 * @return
 */
inline Vector4f createVector4f(const float x, const float y, const float z, const float w) {
    Vector4f out{};
#if defined(USE_SIMD)
    out.data = _mm_setr_ps(x, y, z, w);
#else
    out.x = x;
    out.y = y;
    out.z = z;
    out.w = w;
#endif
    return out;
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
/**
 * @brief Compares 2 vector4s.
 * @param a
 * @param b
 * @param tolerance How much the value can be off before its considered not equal.
 * @return True if the vectors are equal.
 */
inline bool compareVectors(Vector4f a, const Vector4f b, const float tolerance) {
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
inline bool operator==(const Vector4f& left, const Vector4f& right) {
    return compareVectors(left, right, FF_EPSILON);
}

/**
 * @brief Vector4 used for rotation.
 */
typedef Vector4f Quat;

/**
 * @brief Multiplies 2 quats together
 * @param a
 * @param b
 * @return The resulting quat.
 */
inline Quat multiplyQuat(const Quat a, const Quat b) {
    Quat out;

    out.x = a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x;
    out.y = -a.x * b.z + a.y * b.w + a.z * b.x + a.w * b.y;
    out.z = a.x * b.y - a.y * b.x + a.z * b.w + a.w * b.z;
    out.w = -a.x * b.x - a.y * b.y - a.z * b.z + a.w * b.w;

    return out;
}

/**
 * @brief Creates a default quat rotation.
 * @return
 */
inline Quat quatIdentity() {
    return Quat{0, 0, 0, 1};
}

union FOXFIRE_API Mat4 {
    alignas(16) float data[16];
#if defined(USE_SIMD)
    Vector4f rows[4];
#endif
};

/**
 * @brief Creates a default Mat4.
 * @return
 */
inline Mat4 matrixIdentity() {
    Mat4 out{};
    out.data[0] = 1;
    out.data[5] = 1;
    out.data[10] = 1;
    out.data[15] = 1;
    return out;
}
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

/**
 * @brief
 * @param vector
 * @param matrix
 * @return
 */
inline Vector3f transformVector3(const Vector3f vector, const Mat4 &matrix) {
    Vector3f out{};
    out.x = vector.x * matrix.data[0 + 0] + vector.y * matrix.data[4 + 0] + vector.z * matrix.data[8 + 0] + 1.0f * matrix.data[12 + 0];
    out.y = vector.x * matrix.data[0 + 1] + vector.y * matrix.data[4 + 1] + vector.z * matrix.data[8 + 1] + 1.0f * matrix.data[12 + 1];
    out.z = vector.x * matrix.data[0 + 2] + vector.y * matrix.data[4 + 2] + vector.z * matrix.data[8 + 2] + 1.0f * matrix.data[12 + 2];
    return out;
}

struct Vertex {};

/**
 * @brief Contains data for a 3d vertex
 */
struct Vertex3d : Vertex{
    /**
     * @brief Position of the vertex.
     */
    Vector3f position;
    /**
     * @brief The direction the vertex is facing (for lighting).
     */
    Vector3f normal;
    /**
     * @brief Coordinate in the texture to take color from.
     */
    Vector2f textureCoordinate;
    /**
     * @brief The color of this vertex.
     */
    Vector4f color;
    /**
     * @brief
     */
    Vector4f tangent;
};
inline bool operator==(const Vertex3d& left, const Vertex3d& right) {
    return compareVectors(left.position, right.position, FF_EPSILON) &&
        compareVectors(left.normal, right.normal, FF_EPSILON) &&
            compareVectors(left.textureCoordinate, right.textureCoordinate, FF_EPSILON) &&
                compareVectors(left.color, right.color, FF_EPSILON) &&
                    compareVectors(left.tangent, right.tangent, FF_EPSILON);
}

/**
 * @brief Data for a 2d vertex
 */
struct Vertex2d : Vertex{
    /**
     * @brief Position of the vertex.
     */
    Vector2f position;
    /**
     * @brief Coordinate in the texture to take color from.
     */
    Vector2f textureCoordinate;
};

/**
 * @brief How far the object's vertices go from the center.
 */
struct Extent2D {
    Vector2f min{};
    Vector2f max{};
};

/**
 * @brief How far the object's vertices go from the center.
 */
struct Extent3D {
    Vector3f min{};
    Vector3f max{};
};

/**
 * @brief Determines if a number is a power of 2.
 * @param value The number to check.
 * @return True if the number is a power of 2.
 */
inline bool isPowerOfTwo(const unsigned long value) {return value != 0 && ((value & (value - 1)) == 0);}
//Returns the squared length of the vector
//This should be used to compare lengths because sqrt is expensive.
//Do the other getter if you need the real length
/**
 * @brief Gets the squared length of a vector. Use this when comparing lengths instead of the sqrt function.
 * @param vector
 * @return
 */
inline float getLengthSquared(const Vector2f& vector) {return vector.x * vector.x + vector.y * vector.y;}
/**
 * @brief Gets the squared length of a vector. Use this when comparing lengths instead of the sqrt function.
 * @param vector
 * @return
 */
inline float getLengthSquared(const Vector3f& vector) {return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;}
/**
 * @brief Gets the squared length of a vector. Use this when comparing lengths instead of the sqrt function.
 * @param vector
 * @return
 */
inline float getLengthSquared(const Vector4f& vector) {return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;}
/**
 * @brief Converts a vector4 to a vector3.
 * @param vector4
 * @return
 */
inline Vector3f toVector3f(const Vector4f& vector4) {return Vector3f{vector4.x, vector4.y, vector4.z};}

/**
 * @brief Converts a vector3 to a vector4.
 * @param vector3
 * @param w The value for the 4th argument in the vector4.
 * @return
 */
inline Vector4f toVector4f(const Vector3f& vector3, const float w) {
#if defined(USE_SIMD)
    return Vector4f{_mm_setr_ps(vector3.x, vector3.y, vector3.z, w)};
#else
    return Vector4f{vector3.x, vector3.y, vector3.z, w};
#endif
}

/**
 * @brief Converts a string to a vector4.
 * @param string
 * @param out
 * @return False if the string cannot be converted.
 */
inline bool stringToVector4f(const String &string, Vector4f& out) {
    FF_Memory::ff_clear(&out, sizeof(Vector4f));
    const int result = sscanf(string.c_str(), "%f %f %f %f", &out.x, &out.y, &out.z, &out.w);
    return result != -1;
}

/**
 * @brief Converts a vector4 to a string.
 * @param outString
 * @param vector
 */
inline void vector4fToString(String& outString, const Vector4f vector) {
    outString.clear();
    outString.append(std::to_string(vector.x));
    outString.append(" ");
    outString.append(std::to_string(vector.y));
    outString.append(" ");
    outString.append(std::to_string(vector.z));
    outString.append(" ");
    outString.append(std::to_string(vector.w));
}

/**
 * @brief Converts a string to a vector3.
 * @param string
 * @param out
 * @return False if the string cannot be converted.
 */
inline bool stringToVector3f(const String &string, Vector3f& out) {
    FF_Memory::ff_clear(&out, sizeof(Vector3f));
    const int result = sscanf(string.c_str(), "%f %f %f", &out.x, &out.y, &out.z);
    return result != -1;
}

/**
 * @brief Converts a string to a vector2.
 * @param string
 * @param out
 * @return False if the string cannot be converted.
 */
inline bool stringToVector2f(const String &string, Vector2f& out) {
    FF_Memory::ff_clear(&out, sizeof(Vector2f));
    const int result = sscanf(string.c_str(), "%f %f", &out.x, &out.y);
    return result != -1;
}

//Returns the real length of a vector
//EXPENSIVE
/**
 * @brief Gets the length of the vector. Use getLengthSquared() instead of this when comparing lengths because this is expensive.
 * @param vector
 * @return
 */
inline float getVectorLength(const Vector2f& vector) {return FF_Math::sqrt(getLengthSquared(vector));}
/**
 * @brief Gets the length of the vector. Use getLengthSquared() instead of this when comparing lengths because this is expensive.
 * @param vector
 * @return
 */
inline float getVectorLength(const Vector3f& vector) {return FF_Math::sqrt(getLengthSquared(vector));}
/**
 * @brief Gets the length of the vector. Use getLengthSquared() instead of this when comparing lengths because this is expensive.
 * @param vector
 * @return
 */
inline float getVectorLength(const Vector4f& vector) {return FF_Math::sqrt(getLengthSquared(vector));}

/**
 * @brief Returns the distance between 2 vectors.
 * @param a
 * @param b
 * @return
 */
inline float getVectorDistance(const Vector2f a, const Vector2f b) {return getVectorLength(a - b);}
/**
 * @brief Returns the distance between 2 vectors.
 * @param a
 * @param b
 * @return
 */
inline float getVectorDistance(const Vector3f a, const Vector3f b) {return getVectorLength(a - b);}

/**
 * @brief Returns the dot product of 2 vectors.
 * @param a
 * @param b
 * @return
 */
inline float getVectorDotProduct(const Vector3f a, const Vector3f b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * @brief Returns the cross product of 2 vectors.
 * @param a
 * @param b
 * @return
 */
inline Vector3f getVectorCrossProduct(const Vector3f a, const Vector3f b) {
    return Vector3f{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
/**
 * @brief Returns the dot product of 2 vectors.
 * @param a
 * @param b
 * @return
 */
inline float getVectorDotProduct(Vector4f a, const Vector4f b) {
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

/**
 * @brief Normalizes a vector.
 * @param vector
 */
inline void normalize(Vector2f* vector) {
    const float lengthSq = getLengthSquared(*vector);
    if (lengthSq <= FF_EPSILON) {
        *vector = zeroVector2f();
        return;
    }
    const float length = 1.0f / FF_Math::sqrt(lengthSq);
    vector->x *= length;
    vector->y *= length;
}
/**
 * @brief Normalizes a vector.
 * @param vector
 */
inline void normalize(Vector2f& vector) {
    normalize(&vector);
}
/**
 * @brief Normalizes a vector.
 * @param vector
 */
inline void normalize(Vector3f* vector) {
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
/**
 * @brief Normalizes a vector.
 * @param vector
 */
inline void normalize(Vector3f& vector) {
    normalize(&vector);
}
/**
 * @brief Normalizes a vector.
 * @param vector
 */
inline void normalize(Vector4f* vector) {
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
/**
 * @brief Normalizes a vector.
 * @param vector
 */
inline void normalize(Vector4f& vector) {
    normalize(&vector);
}

/**
 * @brief Generates an orthographic view.
 * @param left
 * @param right
 * @param bottom
 * @param top
 * @param ffNear
 * @param ffFar
 * @return
 */
inline Mat4 orthographic(const float left, const float right, const float bottom, const float top, const float ffNear, const float ffFar) {
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

/**
 * @brief Generates a perspective view
 * @param fovRadians Angle of the FOV.
 * @param aspectRatio Viewport aspect ratio.
 * @param ffNear How close an object can get before it is culled.
 * @param ffFar How far an object can get before it is culled.
 * @return
 */
inline Mat4 perspective(const float fovRadians, const float aspectRatio, const float ffNear, const float ffFar) {
    const float halfTanFov = FF_Math::tan(fovRadians * 0.5f);
    Mat4 out{};
    out.data[0] = 1.0f / (aspectRatio * halfTanFov);
    out.data[5] = 1.0f / halfTanFov;
    out.data[10] = ffFar / (ffNear - ffFar);
    out.data[11] = -1.0f;
    out.data[14] = (ffFar * ffNear) / (ffNear - ffFar);
    return out;
}

//Creates a view matrix looking from pos towards target
/**
 * @brief Generates a view matrix looking from pos towards a target.
 * @param pos Position of the view
 * @param target Position to look at.
 * @param up The up vector of the view.
 * @return
 */
inline Mat4 lookAt(Vector3f pos, Vector3f target, Vector3f up) {
    Mat4 out{};
    Vector3f zAxis = target - pos;
    normalize(zAxis);
    Vector3f xAxis = getVectorCrossProduct(zAxis, up);

    if (getLengthSquared(xAxis) <= FF_EPSILON) {
        up = FF_Math::abs(zAxis.y) > 0.999f ? rightVector3f() : upVector3f();
        xAxis = getVectorCrossProduct(zAxis, up);
    }

    normalize(xAxis);
    Vector3f yAxis = getVectorCrossProduct(xAxis, zAxis);

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
    out.data[12] = -getVectorDotProduct(xAxis, pos);
    out.data[13] = -getVectorDotProduct(yAxis, pos);
    out.data[14] = getVectorDotProduct(zAxis, pos);
    out.data[15] = 1.0f;
    return out;
}

/**
 * @brief Inverts a matrix.
 * @param matrix
 * @return
 */
inline Mat4 invertMatrix(const Mat4 &matrix) {
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
//Represents a position change (ie translation).
/**
 * @brief Creates a matrix representing a position change.
 * @param pos
 * @return
 */
inline Mat4 createTranslationMatrix(const Vector3f& pos) {
    Mat4 out = matrixIdentity();
    out.data[12] = pos.x;
    out.data[13] = pos.y;
    out.data[14] = pos.z;
    return out;
}
//Represents a scale change.
/**
 * @brief Creates a matrix representing a scale change.
 * @param scale
 * @return
 */
inline Mat4 createScaleMatrix(const Vector3f& scale) {
    Mat4 out = matrixIdentity();
    out.data[0] = scale.x;
    out.data[5] = scale.y;
    out.data[10] = scale.z;
    return out;
}

/**
 * @brief Creates a matrix representing a rotation change.
 * @param radians
 * @return
 */
inline Mat4 createEulerXMatrix(const float radians) {
    Mat4 out = matrixIdentity();
    const float c = FF_Math::cos(radians);
    const float s = FF_Math::sin(radians);

    out.data[5] = c;
    out.data[6] = s;
    out.data[9] = -s;
    out.data[10] = c;
    return out;
}

/**
 * @brief Creates a matrix representing a rotation on the y axis.
 * @param radians
 * @return
 */
inline Mat4 createEulerYMatrix(const float radians) {
    Mat4 out = matrixIdentity();
    const float c = FF_Math::cos(radians);
    const float s = FF_Math::sin(radians);

    out.data[0] = c;
    out.data[2] = -s;
    out.data[8] = s;
    out.data[10] = c;
    return out;
}

/**
 * @brief Creates a matrix representing a rotation on the z axis.
 * @param radians
 * @return
 */
inline Mat4 createEulerZMatrix(const float radians) {
    Mat4 out = matrixIdentity();
    const float c = FF_Math::cos(radians);
    const float s = FF_Math::sin(radians);

    out.data[0] = c;
    out.data[1] = s;
    out.data[4] = -s;
    out.data[5] = c;
    return out;
}

/**
 * @brief Creates a rotation from the given values.
 * @param x
 * @param y
 * @param z
 * @return
 */
inline Mat4 createEuler(const float x, const float y, const float z) {
    const Mat4 rx = createEulerXMatrix(x);
    const Mat4 ry = createEulerYMatrix(y);
    const Mat4 rz = createEulerZMatrix(z);
    const Mat4 out = rx * ry * rz;
    return out;
}

/**
 * @brief Creates a rotation out of a vector.
 * @param rotation
 * @return
 */
inline Mat4 createEuler(const Vector3f rotation) {
    const Mat4 rx = createEulerXMatrix(rotation.x);
    const Mat4 ry = createEulerYMatrix(rotation.y);
    const Mat4 rz = createEulerZMatrix(rotation.z);
    const Mat4 out = rx * ry * rz;
    return out;
}

/**
 * @brief
 * @param matrix
 * @return
 */
inline Mat4 transposeMatrix(const Mat4 &matrix) {
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

/**
 * @brief Returns a vector representing the backwards direction of the matrix.
 * @param matrix
 * @return
 */
inline Vector3f getBackwardDirection(const Mat4 &matrix) {
    Vector3f forward{};
    forward.x = matrix.data[2];
    forward.y = matrix.data[6];
    forward.z = matrix.data[10];
    normalize(forward);
    return forward;
}
/**
 * @brief Returns a vector representing the forwards direction of the matrix.
 * @param matrix
 * @return
 */
inline Vector3f getForwardDirection(const Mat4 &matrix) {
    return getBackwardDirection(matrix) * -1.0f;
}
/**
 * @brief Returns a vector representing the up direction of the matrix.
 * @param matrix
 * @return
 */
inline Vector3f getUpDirection(const Mat4 &matrix) {
    Vector3f up{};
    up.x = matrix.data[1];
    up.y = matrix.data[5];
    up.z = matrix.data[9];
    normalize(up);
    return up;
}
/**
 * @brief Returns a vector representing the down direction of the matrix.
 * @param matrix
 * @return
 */
inline Vector3f getDownDirection(const Mat4 &matrix) {
    return getUpDirection(matrix) * -1.0f;
}
/**
 * @brief Returns a vector representing the right direction of the matrix.
 * @param matrix
 * @return
 */
inline Vector3f getRightDirection(const Mat4 &matrix) {
    Vector3f right{};
    right.x = matrix.data[0];
    right.y = matrix.data[4];
    right.z = matrix.data[8];
    normalize(right);
    return right;
}
/**
 * @brief Returns a vector representing the left direction of the matrix.
 * @param matrix
 * @return
 */
inline Vector3f getLeftDirection(const Mat4 &matrix) {
    return getRightDirection(matrix) * -1.0f;
}

/**
 * @brief
 * @param q
 * @return
 */
inline float getQuatNormal(const Quat q) {
    return FF_Math::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

/**
 * @brief Normalizes a quaternion.
 * @param q
 * @return
 */
inline Quat normalizeQuat(const Quat q) {
    const float normal = getQuatNormal(q);
    return Quat{q.x / normal, q.y / normal, q.z / normal, q.w / normal};
}

/**
 * @brief
 * @param q
 * @return
 */
inline Quat getQuatConjugate(const Quat q) {
    return Quat{-q.x, -q.y, -q.z, q.w};
}

/**
 * @brief
 * @param q
 * @return
 */
inline Quat getQuatInverse(const Quat q) {
    return normalizeQuat(getQuatConjugate(q));
}

/**
 * @brief Gets the dot product of 2 quats.
 * @param a
 * @param b
 * @return
 */
inline float getQuatDotProduct(const Quat a, const Quat b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/**
 * @brief Converts a quart to a matrix.
 * @param q
 * @return
 */
inline Mat4 convertQuatToMatrix(const Quat q) {
    Mat4 out = matrixIdentity();

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

/**
 * @brief Converts a quat to a rotation matrix
 * @param q
 * @param center
 * @return
 */
inline Mat4 convertQuatToRotationMatrix(const Quat q, const Vector3f center) {
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

/**
 * @brief Returns a quart from an angle.
 * @param axis Axis of the angle.
 * @param angle Angle of the quat.
 * @param shouldNormalize Whether the quat should be normalized before returning.
 * @return
 */
inline Quat getQuatFromAxisAngle(const Vector3f axis, const float angle, const bool shouldNormalize) {
    const float halfAngle = 0.5f * angle;
    const float s = FF_Math::sin(halfAngle);
    const float c = FF_Math::cos(halfAngle);
    const auto q = Quat{s * axis.x, s * axis.y, s * axis.z, c};
    if (shouldNormalize) {
        return normalizeQuat(q);
    }
    return q;
}

/**
 * @brief
 * @param a
 * @param b
 * @param percentage
 * @return
 */
inline Quat slerpQuat(Quat a, Quat b, float percentage) {
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

/**
 * @brief Converts degrees to radians.
 * @param degrees
 * @return
 */
inline float degreesToRadians(const float degrees) {
    return degrees * FF_DEGREE_TO_RADIAN_MULTIPLIER;
}

/**
 * @brief Converts radians to degrees.
 * @param radians
 * @return
 */
inline float RadiansToDegrees(const float radians) {
    return radians * FF_RADIAN_TO_DEGREE_MULTIPLIER;
}