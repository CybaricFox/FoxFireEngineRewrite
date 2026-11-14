//
// Created by cmorg on 11/14/2026.
//

#pragma once

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