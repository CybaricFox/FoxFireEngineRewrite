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

#include "foxfire_export.h"

/**
 * @brief Handles system time calculations
 */
class FOXFIRE_API Clock {
private:
    double startTime = 0;
    double elapsedTime = 0;

public:
    void update();
    void start();
    void stop();

    [[nodiscard]] double getElapsedTime() const {return elapsedTime;}
};
