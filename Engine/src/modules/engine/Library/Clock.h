/**
*   @file Clock.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "../Core/Platform.h"


/**
 * @brief Handles system time calculations
 */
class Clock {
private:
    double startTime = 0;
    double elapsedTime = 0;

public:
    void update(const Platform& platform);
    void start(const Platform& platform);
    void stop();

    [[nodiscard]] double getElapsedTime() const {return elapsedTime;}
};
