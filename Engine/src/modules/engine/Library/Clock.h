//
// Created by cmorg on 7/1/2026.
//

#pragma once
#include "../Core/Platform.h"


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
