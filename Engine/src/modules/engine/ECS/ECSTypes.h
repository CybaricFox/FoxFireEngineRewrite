//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "foxfire_export.h"
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"

/**
 * @brief Base class for entity components. Tracks component size.
 */
struct EntityComponent {
    unsigned int componentId = INVALID_ID_U32;

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
 * @brief This class holds the current id to use for registering components. It should only be used with the component registry.
 */
class internalComponentRegister {
protected:
    inline static unsigned int currentComponentId = 0;
};

/**
 * @brief Each component has its own register in static storage. The register holds the id for that component without the need for a virtual lookup.
 * @tparam T A component class
 */
template<typename T>
requires std::derived_from<T, EntityComponent>
class FOXFIRE_API ComponentRegister : public internalComponentRegister {
private:
    //This is the id for each component
    inline static unsigned int componentId = INVALID_ID_U32;

public:
    static void registerComponent() {
        if (componentId != INVALID_ID_U32) {
            Logger::logWarn("Component " + toString(componentId) + " is already registered.");
            return;
        }

        currentComponentId++;
        componentId = currentComponentId;
    }

    static unsigned int getComponentId() {
        if (componentId == INVALID_ID_U32) {
            Logger::logError("Component id is invalid!");
        }
        return componentId;
    }
};

/**
 * @brief Wrapper for entity components. Automates construction of components.
 * @tparam T Type of component
 */
template <typename T>
struct EntityComponentWrapper : EntityComponent {
    EntityComponentWrapper() {
        componentId = ComponentRegister<T>::getComponentId();
        if (componentId == INVALID_ID_U32) {
            Logger::logError("Internal component id is invalid!");
        }
    }
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