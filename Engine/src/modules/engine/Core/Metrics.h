//
// Created by cmorg on 11/10/2026.
//

#pragma once

#include "foxfire_export.h"

/**
 *  @file Metrics.h
 *  @layer Engine
 *  @module Core
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/10/2026
 *
 *  @copyright (c) 2026
 */

#define AVERAGE_COUNT 30 //How many frames should pass before taking the average frames per ms?

class FOXFIRE_API Metrics {
private:
    unsigned char frameAverageCounter = 0;
    double msTimes[AVERAGE_COUNT]{};
    double msAverage = 0;
    int frames = 0;
    double accumulatedFrames = 0;
    double fps = 0;

public:
    [[nodiscard]] double getFPS() const {return fps;}
    [[nodiscard]] double calaculateFrameTime() const {return msAverage;}
    void getFrameMetrics(double& outFPS, double& outFrameTime) const;

    void update(double frameElapsedTime);
};