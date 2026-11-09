//
// Created by cmorg on 8/12/2026.

#pragma once
#include "src/modules/engine/ECS/ECSTypes.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

struct Mesh final : EntityComponentWrapper<Mesh> {
    unsigned int id = INVALID_ID_U32;
    unsigned short geometryCount = 0;
    DynamicArray<Geometry*> geometries{};
    unsigned char generation = INVALID_ID_U8;

    Mesh();
    ~Mesh() override;

    Mesh(const Mesh& other);

    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;
};
