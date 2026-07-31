//
// Created by cmorg on 7/28/2026.
//

#pragma once
#include "src/defines.h"

struct Texture {
    unsigned int id = INVALID_ID;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char channelCount = 0;
    bool bIsTransparent = false;
    unsigned int generation = INVALID_ID;
    void* data = nullptr;
};
