//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"

/**
 * @brief Base class for entity components. Tracks component size.
 */
struct EntityComponent {
    virtual ~EntityComponent() = default;

    /**
     * @brief Returns the size of this component
     * @return The size of this component.
     */
    virtual unsigned long getComponentSize() = 0;

    /**
     * @brief Copies the memory of this component to the given location.
     * @param destination Destination in memory to copy to.
     * @return The copy of this component.
     */
    virtual EntityComponent* copyTo(void* destination) const = 0;
};

/**
 * @brief Wrapper for entity components. Automates construction of components.
 * @tparam T Type of component
 */
template <typename T>
struct EntityComponentWrapper : EntityComponent {
    /**
     * @brief Returns the size of this component
     * @return The size of this component.
     */
    unsigned long getComponentSize() override {
        return sizeof(T);
    }
    /**
     * @brief Copies the memory of this component to the given location.
     * @param destination Destination in memory to copy to.
     * @return The copy of this component.
     */
    EntityComponent* copyTo(void* destination) const override {
        return std::construct_at(static_cast<T*>(destination), static_cast<const T &>(*this));
    }
};

/**
 * @brief An Entity is a dynamic array of components. This array will be used as a template to create instances from.
 */
struct Entity {
    /**
     * @brief Array of components tied to this entity.
     */
    DynamicArray<EntityComponent*> components{};
};

/**
 * @brief An instance of an entity.
 */
struct EntityInstance {
    /**
     * @brief id of this entity.
     */
    unsigned int id = INVALID_ID_U32;
    /**
     * @brief Number of components that this entity has.
     */
    unsigned char componentCount = 0;
    /**
     * @brief Size of this instance + its components.
     */
    unsigned long totalSize = 0;
};