//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"

struct EntityComponent {
    unsigned int componentId = INVALID_ID_U32;
    unsigned long componentSize = 0;

    virtual ~EntityComponent() = default;
};

struct Entity {
    DynamicArray<EntityComponent*> components{};
};

struct EntityInstance {
    unsigned int id = INVALID_ID_U32;
    unsigned char componentCount = 0;
    unsigned long totalSize = 0;
};