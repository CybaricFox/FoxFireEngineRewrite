//
// Created by cmorg on 8/11/2026.
//

#pragma once
#include "FF_Math.h"

/**
 *  @file GeometryUtils.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/11/2026
 *
 *  @copyright (c) 2026
 */

/**
 * @brief Collection of Geometry utility functions
 */
class GeometryUtils {
public:
    static void generateNormals(unsigned int vertexCount, Vertex3d* vertices, unsigned int indexCount, const unsigned int* indices);

    static void generateTangents(unsigned int vertexCount, Vertex3d* vertices, unsigned int indexCount, const unsigned int* indices);
};