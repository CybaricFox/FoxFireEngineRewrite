//
// Created by cmorg on 8/4/2026.
//

#pragma once
#include "src/modules/engine/Library/FF_Math.h"

//Must be 256 bytes!
struct GlobalUniform {
    Mat4 projection; //64 bytes
    Mat4 view; //64 bytes
    Mat4 reserved0; //64 bytes
    Mat4 reserved1; //64 bytes
};

//64 bytes
struct alignas(256) MaterialUniform {
    Vector4f diffuse; //16 bytes
    Vector4f reserved0;
    Vector4f reserved1;
    Vector4f reserved2;
};