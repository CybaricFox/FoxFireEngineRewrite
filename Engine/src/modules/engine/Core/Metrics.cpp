//
// Created by cmorg on 11/10/2026.
//

#include "Metrics.h"

void Metrics::getFrameMetrics(double &outFPS, double &outFrameTime) const {
    outFPS = fps;
    outFrameTime = msAverage;
}

void Metrics::update(const double frameElapsedTime) {
    const double frameMs = frameElapsedTime * 1000;
    msTimes[frameAverageCounter] = frameMs;

    if (frameAverageCounter == AVERAGE_COUNT - 1) {
        for (const double msTime : msTimes) {
            msAverage += msTime;
        }

        msAverage /= AVERAGE_COUNT;
    }

    frameAverageCounter++;
    frameAverageCounter %= AVERAGE_COUNT;

    accumulatedFrames += frameMs;

    if (accumulatedFrames > 1000) {
        fps = frames;
        accumulatedFrames -= 1000;
        frames = 0;
    }

    frames++;
}
