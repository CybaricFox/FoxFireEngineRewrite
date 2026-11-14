//
// Created by cmorg on 11/14/2026.
//

#pragma once

#include "foxfire_export.h"
#include "Matrix.h"
#include "Vectors.h"

/**
 *  @file MatrixUtils.h
 *  @layer 
 *  @module
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/14/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API MatrixUtils {
public:
    /**
        * @brief Creates a default Mat4.
        * @return
    */
    static Mat4 matrixIdentity();
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
    static Mat4 orthographic(float left, float right, float bottom, float top, float ffNear, float ffFar);

    /**
     * @brief Generates a perspective view
     * @param fovRadians Angle of the FOV.
     * @param aspectRatio Viewport aspect ratio.
     * @param ffNear How close an object can get before it is culled.
     * @param ffFar How far an object can get before it is culled.
     * @return
     */
    static Mat4 perspective(float fovRadians, float aspectRatio, float ffNear, float ffFar);

    //Creates a view matrix looking from pos towards target
    /**
     * @brief Generates a view matrix looking from pos towards a target.
     * @param pos Position of the view
     * @param target Position to look at.
     * @param up The up vector of the view.
     * @return
     */
    static Mat4 lookAt(Vector3f pos, Vector3f target, Vector3f up);
    /**
 * @brief Inverts a matrix.
 * @param matrix
 * @return
 */
    static Mat4 invertMatrix(const Mat4 &matrix);
    //Represents a position change (ie translation).
    /**
     * @brief Creates a matrix representing a position change.
     * @param pos
     * @return
     */
    static Mat4 createTranslationMatrix(const Vector3f& pos);
    //Represents a scale change.
    /**
     * @brief Creates a matrix representing a scale change.
     * @param scale
     * @return
     */
    static Mat4 createScaleMatrix(const Vector3f& scale);

    /**
     * @brief Creates a matrix representing a rotation change.
     * @param radians
     * @return
     */
    static Mat4 createEulerXMatrix(float radians);

    /**
     * @brief Creates a matrix representing a rotation on the y axis.
     * @param radians
     * @return
     */
    static Mat4 createEulerYMatrix(float radians);

    /**
     * @brief Creates a matrix representing a rotation on the z axis.
     * @param radians
     * @return
     */
    static Mat4 createEulerZMatrix(float radians);
    /**
 * @brief Creates a rotation from the given values.
 * @param x
 * @param y
 * @param z
 * @return
 */
    static Mat4 createEuler(float x, float y, float z);

    /**
     * @brief Creates a rotation out of a vector.
     * @param rotation
     * @return
     */
    static Mat4 createEuler(Vector3f rotation);

    /**
     * @brief
     * @param matrix
     * @return
     */
    static Mat4 transposeMatrix(const Mat4 &matrix);
    /**
 * @brief Converts a quat to a rotation matrix
 * @param q
 * @param center
 * @return
 */
    static Mat4 convertQuatToRotationMatrix(Quat q, Vector3f center);
};