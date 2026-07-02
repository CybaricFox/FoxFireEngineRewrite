//
// Created by cmorg on 7/1/2026.
//

#include "Clock.h"

#include "../Core/Platform.h"

void Clock::update(const Platform& platform) {
    if (startTime != 0) {
        elapsedTime = platform.getAbsoluteTime() - startTime;
    }
}

void Clock::start(const Platform& platform) {
    startTime = platform.getAbsoluteTime();
    elapsedTime = 0;
}

void Clock::stop() {
    startTime = 0;
}
