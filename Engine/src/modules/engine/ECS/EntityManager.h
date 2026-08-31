//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "ECSTypes.h"
#include "src/modules/engine/Memory/DynamicArray.h"

/**
 *  @file Entity.h
 *  @layer 
 *  @module
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/12/2026
 *
 *  @copyright (c) 2026
 */

class EntityManager {
private:
    DynamicAllocator allocator{};
    DynamicArray<EntityInstance*> instances{};
    void* memory = nullptr;
    unsigned long memorySize = 0;
    unsigned int count = 0;

public:
    EntityManager();
    ~EntityManager();

    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    EntityManager(EntityManager&&) noexcept = default;
    EntityManager& operator=(EntityManager&&) noexcept = default;

    [[nodiscard]] unsigned int getEntityCount() const {return count;}

    void copyFromTemplate(Entity& entity, unsigned int id);

    /**
     * @brief Checks if a component exists and returns it
     * @tparam T The component type
     * @param id id of the entity
     * @return nullptr if component does not exist or a pointer to the target component
     */
    template<typename T>
    requires std::derived_from<T, EntityComponent>
    T* getComponent(const unsigned int id) {
        for (EntityInstance* instance : instances) {
            if (instance->id == id) {
                //Get the first component
                auto component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));
                unsigned char componentCount = 1;
                //Check every component
                while (componentCount <= instance->componentCount) {
                    //Check if the component is of the correct type
                    T* result = dynamic_cast<T*>(component);
                    if (result != nullptr) {
                        return result;
                    }

                    component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(component) + component->getComponentSize());
                    componentCount++;
                }

                //Component does not exist. Do not log.
                return nullptr;
            }
        }

        Logger::logWarn("getComponent reached the end of allocation without finding the entity: " + std::to_string(id) + ". This message should never appear.");
        return nullptr;
    }
};