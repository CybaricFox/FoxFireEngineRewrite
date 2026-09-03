//
// Created by cmorg on 8/12/2026.

#pragma once
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

struct Mesh final : EntityComponentWrapper<Mesh> {
    unsigned short geometryCount = 0;
    DynamicArray<Geometry*> geometries{};

    Mesh() = default;
    ~Mesh() override {
        geometries.shutdown();
    }

    Mesh(const Mesh& other)
        : geometryCount(other.geometryCount) {

        if (!other.geometries.isEmpty()) {
            geometries.initialize(other.geometries.getLength());

            for (Geometry* geometry : other.geometries) {
                geometries.push(geometry);
            }
        }
    }

    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;
};
