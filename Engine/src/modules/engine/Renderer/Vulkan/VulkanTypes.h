/**
*   @file VulkanTypes.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "src/modules/engine/Library/FF_Math.h"

/**
 * @brief 256 bytes. Used for camera projection and view.
 */
struct GlobalUniform {
    Mat4 projection; //64 bytes
    Mat4 view; //64 bytes
    Mat4 reserved0; //64 bytes
    Mat4 reserved1; //64 bytes
};

/**
 * @brief 256 bytes. Contains instance color.
 */
struct alignas(256) MaterialUniform {
    Vector4f diffuse; //16 bytes
    Vector4f reserved0;
    Vector4f reserved1;
    Vector4f reserved2;
};