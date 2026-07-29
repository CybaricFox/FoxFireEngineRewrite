//
// Created by cmorg on 7/28/2026.
//

#pragma once

struct Texture {
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned char channelCount;
    bool bIsTransparent;
    unsigned int generation; //Maybe change this to bIsDirty later on.
    void* data;
};