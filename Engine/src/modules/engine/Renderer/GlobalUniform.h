//
// Created by cmorg on 7/17/2026.
//

#pragma once
#include "src/modules/engine/Library/FF_Math.h"
#include "src/modules/engine/Resources/Resource_Types.h"

//Must be 256 bytes!
struct GlobalUniform {
    Mat4 projection; //64 bytes
    Mat4 view; //64 bytes
    Mat4 reserved0; //64 bytes
    Mat4 reserved1; //64 bytes
};

//64 bytes
struct EntityUniform {
    Vector4f diffuse; //16 bytes
    Vector4f reserved0;
    Vector4f reserved1;
    Vector4f reserved2;
};

struct GeometryRenderData {
    unsigned int id;
    Mat4 model;
    Texture* textures[16];
};