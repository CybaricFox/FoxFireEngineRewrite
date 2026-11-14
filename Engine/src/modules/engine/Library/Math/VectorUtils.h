//
// Created by cmorg on 11/14/2026.
//

#pragma once

#include "foxfire_export.h"
#include "Vectors.h"

/**
 *  @file VectorUtils.h
 *  @layer 
 *  @module
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/14/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API VectorUtils {
public:
    /**
        * @brief Compares 2 vector2s.
        * @param a
        * @param b
        * @param tolerance How much the value can be off before its considered not equal.
        * @return True if the vectors are equal.
    */
    static bool compareVectors(Vector2f a, Vector2f b, float tolerance);
    /**
        * @brief Compares 2 vector3s.
        * @param a
        * @param b
        * @param tolerance How much the value can be off before its considered not equal.
        * @return True if the vectors are equal.
    */
    static bool compareVectors(Vector3f a, Vector3f b,  float tolerance);
    /**
 * @brief Compares 2 vector4s.
 * @param a
 * @param b
 * @param tolerance How much the value can be off before its considered not equal.
 * @return True if the vectors are equal.
 */
    static bool compareVectors(Vector4f a, Vector4f b, float tolerance);
    /**
 * @brief
 * @param vector
 * @param matrix
 * @return
 */
    static Vector3f transformVector3(Vector3f vector, const Mat4 &matrix);

    //Returns the squared length of the vector
    //This should be used to compare lengths because sqrt is expensive.
    //Do the other getter if you need the real length
    /**
     * @brief Gets the squared length of a vector. Use this when comparing lengths instead of the sqrt function.
     * @param vector
     * @return
     */
    static float getLengthSquared(const Vector2f& vector) {return vector.x * vector.x + vector.y * vector.y;}
    /**
     * @brief Gets the squared length of a vector. Use this when comparing lengths instead of the sqrt function.
     * @param vector
     * @return
     */
    static float getLengthSquared(const Vector3f& vector) {return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;}
    /**
     * @brief Gets the squared length of a vector. Use this when comparing lengths instead of the sqrt function.
     * @param vector
     * @return
     */
    static float getLengthSquared(const Vector4f& vector) {return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;}
    /**
     * @brief Converts a vector4 to a vector3.
     * @param vector4
     * @return
     */
    static Vector3f toVector3f(const Vector4f& vector4) {return {vector4.x, vector4.y, vector4.z};}
    /**
 * @brief Converts a vector3 to a vector4.
 * @param vector3
 * @param w The value for the 4th argument in the vector4.
 * @return
 */
    static Vector4f toVector4f(const Vector3f& vector3, const float w) {
#if defined(USE_SIMD)
        return Vector4f{_mm_setr_ps(vector3.x, vector3.y, vector3.z, w)};
#else
        return Vector4f{vector3.x, vector3.y, vector3.z, w};
#endif
    }
    //Returns the real length of a vector
    //EXPENSIVE
    /**
     * @brief Gets the length of the vector. Use getLengthSquared() instead of this when comparing lengths because this is expensive.
     * @param vector
     * @return
     */
    static float getVectorLength(const Vector2f& vector);
    /**
     * @brief Gets the length of the vector. Use getLengthSquared() instead of this when comparing lengths because this is expensive.
     * @param vector
     * @return
     */
    static float getVectorLength(const Vector3f& vector);
    /**
     * @brief Gets the length of the vector. Use getLengthSquared() instead of this when comparing lengths because this is expensive.
     * @param vector
     * @return
     */
    static float getVectorLength(const Vector4f& vector);
    /**
 * @brief Returns the distance between 2 vectors.
 * @param a
 * @param b
 * @return
 */
    static float getVectorDistance(const Vector2f a, const Vector2f b) {return getVectorLength(a - b);}
    /**
     * @brief Returns the distance between 2 vectors.
     * @param a
     * @param b
     * @return
     */
    static float getVectorDistance(const Vector3f a, const Vector3f b) {return getVectorLength(a - b);}
    /**
 * @brief Returns the dot product of 2 vectors.
 * @param a
 * @param b
 * @return
 */
    static float getVectorDotProduct(const Vector3f a, const Vector3f b) {return a.x * b.x + a.y * b.y + a.z * b.z;}

    /**
     * @brief Returns the cross product of 2 vectors.
     * @param a
     * @param b
     * @return
     */
    static Vector3f getVectorCrossProduct(Vector3f a, Vector3f b);
    /**
 * @brief Returns the dot product of 2 vectors.
 * @param a
 * @param b
 * @return
 */
    static float getVectorDotProduct(Vector4f a, Vector4f b);
    /**
 * @brief Normalizes a vector.
 * @param vector
 */
    static void normalize(Vector2f* vector);
    /**
     * @brief Normalizes a vector.
     * @param vector
     */
    static void normalize(Vector2f& vector) {
        normalize(&vector);
    }
    /**
     * @brief Normalizes a vector.
     * @param vector
     */
    static void normalize(Vector3f* vector);
    /**
     * @brief Normalizes a vector.
     * @param vector
     */
    static void normalize(Vector3f& vector) {
        normalize(&vector);
    }
    /**
     * @brief Normalizes a vector.
     * @param vector
     */
    static void normalize(Vector4f* vector);
    /**
     * @brief Normalizes a vector.
     * @param vector
     */
    static void normalize(Vector4f& vector) {
        normalize(&vector);
    }
    /**
 * @brief Returns a vector representing the backwards direction of the matrix.
 * @param matrix
 * @return
 */
    static Vector3f getBackwardDirection(const Mat4 &matrix);
    /**
     * @brief Returns a vector representing the forwards direction of the matrix.
     * @param matrix
     * @return
     */
    static Vector3f getForwardDirection(const Mat4 &matrix) {
        return getBackwardDirection(matrix) * -1.0f;
    }
    /**
     * @brief Returns a vector representing the up direction of the matrix.
     * @param matrix
     * @return
     */
    static Vector3f getUpDirection(const Mat4 &matrix);
    /**
     * @brief Returns a vector representing the down direction of the matrix.
     * @param matrix
     * @return
     */
    static Vector3f getDownDirection(const Mat4 &matrix) {
        return getUpDirection(matrix) * -1.0f;
    }
    /**
     * @brief Returns a vector representing the right direction of the matrix.
     * @param matrix
     * @return
     */
    static Vector3f getRightDirection(const Mat4 &matrix);
    /**
     * @brief Returns a vector representing the left direction of the matrix.
     * @param matrix
     * @return
     */
    static Vector3f getLeftDirection(const Mat4 &matrix) {
        return getRightDirection(matrix) * -1.0f;
    }
};

class FOXFIRE_API QuatUtils {
public:
    /**
        * @brief Multiplies 2 quats together
        * @param a
        * @param b
        * @return The resulting quat.
     */
    static Quat multiplyQuat(Quat a, Quat b);
    /**
        * @brief Creates a default quat rotation.
        * @return
    */
    static Quat quatIdentity() {
        return {0, 0, 0, 1};
    }
    /**
 * @brief
 * @param q
 * @return
 */
    static float getQuatNormal(Quat q);

    /**
     * @brief Normalizes a quaternion.
     * @param q
     * @return
     */
    static Quat normalizeQuat(const Quat q) {
        const float normal = getQuatNormal(q);
        return Quat{q.x / normal, q.y / normal, q.z / normal, q.w / normal};
    }

    /**
     * @brief
     * @param q
     * @return
     */
    static Quat getQuatConjugate(const Quat q) {
        return Quat{-q.x, -q.y, -q.z, q.w};
    }

    /**
     * @brief
     * @param q
     * @return
     */
    static Quat getQuatInverse(const Quat q) {
        return normalizeQuat(getQuatConjugate(q));
    }

    /**
     * @brief Gets the dot product of 2 quats.
     * @param a
     * @param b
     * @return
     */
    static float getQuatDotProduct(const Quat a, const Quat b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    /**
 * @brief Converts a quart to a matrix.
 * @param q
 * @return
 */
    static Mat4 convertQuatToMatrix(Quat q);

    /**
 * @brief Returns a quart from an angle.
 * @param axis Axis of the angle.
 * @param angle Angle of the quat.
 * @param shouldNormalize Whether the quat should be normalized before returning.
 * @return
 */
    static Quat getQuatFromAxisAngle(Vector3f axis, float angle, bool shouldNormalize);
    /**
 * @brief
 * @param a
 * @param b
 * @param percentage
 * @return
 */
static Quat slerpQuat(Quat a, Quat b, float percentage);
};