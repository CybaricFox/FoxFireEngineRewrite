//
// Created by cmorg on 7/1/2026.
//

#include "Clock.h"

#include "../Core/Platform.h"

void Clock::update() {
    if (startTime != 0) {
        elapsedTime = Platform::getAbsoluteTime() - startTime;
    }
}

void Clock::start() {
    startTime = Platform::getAbsoluteTime();
    elapsedTime = 0;
}

void Clock::stop() {
    startTime = 0;
}
