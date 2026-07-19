//
// Created by cmorg on 7/17/2026.
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