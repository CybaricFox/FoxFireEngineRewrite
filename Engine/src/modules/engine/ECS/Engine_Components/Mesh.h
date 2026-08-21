//
// Created by cmorg on 8/12/2026.

#pragma once
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

struct Mesh final : EntityComponent {
    unsigned short geometryCount = 0;
    DynamicArray<Geometry*> geometries{};
    Transform* transform = nullptr;

    Mesh() {
        componentSize = sizeof(Mesh);
    }

    ~Mesh() override {
        geometries.shutdown();
    }
};
