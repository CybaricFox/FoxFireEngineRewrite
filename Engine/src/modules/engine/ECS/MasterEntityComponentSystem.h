//
// Created by cmorg on 8/12/2026.
//

#pragma once
#include "ECSTypes.h"
#include "EntityManager.h"
#include "src/modules/engine/Library/AssetMap.h"
#include "src/modules/engine/Memory/DynamicArray.h"

/**
 *  @file MasterEntityComponentSystem.h
 *  @layer Engine
 *  @module ECS
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/12/2026
 *
 *  @copyright (c) 2026
 */

/**
 * @brief Context used to check what type of entity and id is associated with.
 */
struct EntityContext {
    /**
     * @brief Id of the instance.
     */
    unsigned int id = INVALID_ID_U32;
    /**
     * @brief Name of the associated entity.
     */
    String entityName{};
};

/**
 * @brief Controls entity components
 */
class FOXFIRE_API MasterEntityComponentSystem {
private:
    /**
     * @brief The next id that will be assigned
     */
    unsigned int currentId = -1;

    /**
     * @brief Asset map of entities.
     */
    AssetMap<Entity, AssetContext> templates{};
    /**
     * @brief Asset map of instances.
     */
    static AssetMap<EntityManager, AssetContext>* instances;
    /**
     * @brief Array of entity/instance associations. Sorted by id.
     */
    static DynamicArray<EntityContext>* entities;

    /**
     * @brief
     * @param id Id of the instance.
     * @return Returns the name of the entity associated with the instance id.
     */
    static String getEntityName(unsigned int id);

    /**
     * @brief Gets an unused id and increments it.
     * @return Free id for assignment.
     */
    unsigned int getNewId() {return ++currentId;}

public:
    void initialize();
    void shutdown();

    /**
     * @brief Returns the number of instances associated with this entity.
     * @param name Name of the entity.
     * @return
     */
    unsigned int getEntityCount(const String &name);

    /**
     * @brief Returns an array of instances associated with the given entity.
     * @param type Name of the entity.
     * @return Array of all associated instances.
     */
    DynamicArray<unsigned int> &getAllEntitiesOfType(const String &type);

    /**
     * @brief Creates an instance of an entity.
     * @param name Name of the entity.
     * @return New instance of the entity.
     */
    unsigned int createEntity(const String &name);

    /**
     * @brief Creates a new entity.
     * @param name Name of the entity.
     * @return Pointer to the new entity. Assign components to complete its creation.
     */
    Entity *createEntityType(const String &name);

    /**
     * @brief Gets a component from an instance.
     * @tparam T Type of component
     * @param id id of the instance
     * @return The instance component or nullptr if it doesn't have one.
     */
    template<typename T>
    requires std::derived_from<T, EntityComponent>
    static T* getComponent(const unsigned int id) {
        const String entityName = getEntityName(id);
        if (entityName.empty()) return nullptr;

        EntityManager* manager = instances->getAsset(entityName);

        return manager->getComponent<T>(id);
    }

    /**
     * @brief Adds a component to an instance.
     * @tparam T Type of componet
     * @param id id of the instance
     * @return The newly added component.
     */
    template<typename T>
    requires std::derived_from<T, EntityComponent>
    static T* addComponent(const unsigned int id) {
        const String entityName = getEntityName(id);
        if (entityName.empty()) return nullptr;

        EntityManager* manager = instances->getAsset(entityName);
        return manager->addComponent<T>(id);
    }

    /**
     * @brief Removes a component from an instance.
     * @tparam T Type of component
     * @param id id of the instance.
     */
    template<typename T>
    requires std::derived_from<T, EntityComponent>
    static void removeComponent(const unsigned int id) {
        const String entityName = getEntityName(id);
        if (entityName.empty()) return;

        EntityManager* manager = instances->getAsset(entityName);
        return manager->removeComponent<T>(id);
    }

};