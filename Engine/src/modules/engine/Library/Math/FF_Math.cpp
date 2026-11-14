//
// Created by cmorg on 7/4/2026.
//

#include "FF_Math.h"

#include <cmath>

#include "src/modules/engine/Core/Platform.h"

bool FF_Math::bIsSeeded = false;

void FF_Math::setSeed() {
    srand(static_cast<unsigned int>(Platform::getAbsoluteTime()));
    bIsSeeded = true;
}

float FF_Math::sin(const float value) {
    return sinf(value);
}

float FF_Math::cos(const float value) {
    return cosf(value);
}

float FF_Math::acos(const float value) {
    return acosf(value);
}

float FF_Math::tan(const float value) {
    return tanf(value);
}

float FF_Math::sqrt(const float value) {
    return sqrtf(value);
}

float FF_Math::abs(const float value) {
    return fabsf(value);
}

int FF_Math::randomInt() {
    if (!bIsSeeded) {
        setSeed();
    }
    return rand();
}

float FF_Math::randomFloat() {
    return static_cast<float>(randomInt()) / RAND_MAX;
}

int FF_Math::randomRange(const int min, const int max) {
    if (!bIsSeeded) {
        setSeed();
    }
    return rand() % (max - min + 1) + min;
}

float FF_Math::randomRange(const float min, const float max) {
    return min + static_cast<float>(randomInt()) / (RAND_MAX / (max - min));
}
