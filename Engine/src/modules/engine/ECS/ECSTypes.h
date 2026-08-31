//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"

struct EntityComponent {
    virtual ~EntityComponent() = default;

    virtual unsigned long getComponentSize() = 0;
    virtual EntityComponent* copyTo(EntityComponent* destination) const = 0;
};

template <typename T>
struct EntityComponentWrapper : EntityComponent {
    unsigned long getComponentSize() override {
        return sizeof(T);
    }
    EntityComponent* copyTo(EntityComponent* destination) const override {
        return std::construct_at(static_cast<T*>(destination), static_cast<const T &>(*this));
    }
};

struct Entity {
    DynamicArray<EntityComponent*> components{};
};

struct EntityInstance {
    unsigned int id = INVALID_ID_U32;
    unsigned char componentCount = 0;
    unsigned long totalSize = 0;
};