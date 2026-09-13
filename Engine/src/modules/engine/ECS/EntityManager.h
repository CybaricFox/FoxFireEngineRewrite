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

    EntityManager(EntityManager&&) = delete;
    EntityManager& operator=(EntityManager&&) = delete;

    [[nodiscard]] unsigned int getEntityCount() const {return count;}
    DynamicArray<EntityInstance*>& getInstances() {return instances;}

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

    template<typename T>
    requires std::derived_from<T, EntityComponent>
    T* addComponent(const unsigned int id) {
        for (EntityInstance* instance : instances) {
            if (instance->id == id) {
                unsigned int componentCount = 1;
                auto component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));

                //Check that the component does not already exist
                while (componentCount <= instance->componentCount) {
                    //Check if the component is of the correct type
                    T* result = dynamic_cast<T*>(component);
                    if (result != nullptr) {
                        Logger::logWarn("Attempted to add a component to " + std::to_string(id) + " but that component already exists!");
                        return result;
                    }

                    component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(component) + component->getComponentSize());
                    componentCount++;
                }

                //Create a new entity with the same id but with the new component
                const unsigned long totalSize = instance->totalSize + sizeof(T);
                const auto newComponent = allocator.allocate(totalSize);
                EntityInstance* newInstance = std::construct_at(static_cast<EntityInstance *>(newComponent));
                newInstance->id = instance->id;
                newInstance->totalSize = totalSize;

                //Replace the lookup reference with the new ome
                for (unsigned int i = 0; i < instances.getLength(); i++) {
                    if (instances[i]->id == instance->id) {
                        instances[i] = newInstance;
                        break;
                    }
                }

                auto location = static_cast<unsigned char *>(newComponent) + sizeof(EntityInstance);
                auto oldComponent = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));

                unsigned int newCount = 0;
                while (newCount < instance->componentCount) {
                    const auto copiedComponent = oldComponent->copyTo(location);
                    location += copiedComponent->getComponentSize();
                    oldComponent = reinterpret_cast<EntityComponent*>(reinterpret_cast<unsigned char*>(oldComponent) + oldComponent->getComponentSize());
                    newCount++;
                }

                //Finally add the new component
                auto result = std::construct_at(static_cast<T*>(static_cast<void *>(location)));
                newCount++;

                newInstance->componentCount = newCount;

                //Remove the old instance
                auto componentToRemove = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));
                newCount = 0;
                while (newCount < instance->componentCount) {
                    const unsigned long componentSize = componentToRemove->getComponentSize();
                    std::destroy_at(componentToRemove);
                    componentToRemove = reinterpret_cast<EntityComponent*>(reinterpret_cast<unsigned char*>(componentToRemove) + componentSize);
                    newCount++;
                }

                allocator.free(instance, instance->totalSize);

                return result;
            }
        }

        Logger::logWarn("addComponent reached the end of allocation without finding the entity: " + std::to_string(id) + ". This message should never appear.");
        return nullptr;
    }

    template<typename T>
    requires std::derived_from<T, EntityComponent>
    void removeComponent(const unsigned int id) {
        bool remove = false;
        for (EntityInstance* instance : instances) {
            if (instance->id == id) {
                unsigned int componentCount = 1;
                auto component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));

                //Check that the component exists
                bool found = false;
                while (componentCount <= instance->componentCount) {
                    //Check if the component is of the correct type
                    T* result = dynamic_cast<T*>(component);
                    if (result != nullptr) {
                        found = true;
                        break;
                    }

                    component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(component) + component->getComponentSize());
                    componentCount++;
                }

                if (!found) {
                    Logger::logWarn("Cannot remove a component from " + std::to_string(id) + " because the component could not be found!");
                    return;
                }

                //This deletes the instance if it only had 1 component to begin with. Realistically this should never run because you would have to remove Transform for this to run.
                if (instance->componentCount == 1) {
                    remove = true;
                    break;
                }

                //Create a new entity with the same id but with one less component
                const unsigned long totalSize = instance->totalSize - sizeof(T);
                const auto newComponent = allocator.allocate(totalSize);
                EntityInstance* newInstance = std::construct_at(static_cast<EntityInstance *>(newComponent));
                newInstance->id = instance->id;
                newInstance->totalSize = totalSize;

                //Replace the lookup reference with the new one
                for (unsigned int i = 0; i < instances.getLength(); i++) {
                    if (instances[i]->id == instance->id) {
                        instances[i] = newInstance;
                        break;
                    }
                }

                auto location = static_cast<unsigned char *>(newComponent) + sizeof(EntityInstance);
                auto oldComponent = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));

                unsigned int newCount = 0;
                while (newCount < instance->componentCount) {
                    //Do not copy the removed component
                    if (oldComponent == component) {
                        oldComponent = reinterpret_cast<EntityComponent*>(reinterpret_cast<unsigned char*>(oldComponent) + oldComponent->getComponentSize());
                        newCount++;
                        continue;
                    }

                    const auto copiedComponent = oldComponent->copyTo(location);
                    location += copiedComponent->getComponentSize();
                    oldComponent = reinterpret_cast<EntityComponent*>(reinterpret_cast<unsigned char*>(oldComponent) + oldComponent->getComponentSize());
                    newCount++;
                }

                newInstance->componentCount = instance->componentCount - 1;

                //Remove the old instance
                auto componentToRemove = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));
                newCount = 0;
                while (newCount < instance->componentCount) {
                    const unsigned long componentSize = componentToRemove->getComponentSize();
                    std::destroy_at(componentToRemove);
                    componentToRemove = reinterpret_cast<EntityComponent*>(reinterpret_cast<unsigned char*>(componentToRemove) + componentSize);
                    newCount++;
                }

                allocator.free(instance, instance->totalSize);
                break;
            }
        }

        if (remove) {
            EntityInstance* instance = nullptr;
            unsigned int index = 0;

            for (; index < instances.getLength(); index++) {
                if (instances[index]->id == id) {
                    instance = instances[index];
                    break;
                }
            }

            //Remove the lookup
            instances.pop(index);

            //Remove the old instance
            auto componentToRemove = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));
            unsigned int newCount = 0;
            while (newCount < instance->componentCount) {
                const unsigned long componentSize = componentToRemove->getComponentSize();
                std::destroy_at(componentToRemove);
                componentToRemove = reinterpret_cast<EntityComponent*>(reinterpret_cast<unsigned char*>(componentToRemove) + componentSize);
                newCount++;
            }

            allocator.free(instance, instance->totalSize);
        }
    }

};