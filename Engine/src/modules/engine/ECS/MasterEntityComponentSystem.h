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

struct EntityContext {
    unsigned int id = INVALID_ID_U32;
    String entityName{};
};

class FOXFIRE_API MasterEntityComponentSystem {
private:
    unsigned int currentId = -1;

    AssetMap<Entity, AssetContext> templates{};
    AssetMap<EntityManager, AssetContext> instances{};
    DynamicArray<EntityContext> entities{};

    String getEntityName(unsigned int id);
    unsigned int getNewId() {return ++currentId;}

    Entity *createEntityType(const String &name);
public:
    void initialize();
    void shutdown();

    unsigned int getEntityCount(const String &name);

    unsigned int createEntity(const String &name);

    template<typename T>
    requires std::derived_from<T, EntityComponent>
    T* getComponent(const unsigned int id) {
        const String entityName = getEntityName(id);
        if (entityName.empty()) return nullptr;

        EntityManager* manager = instances.getAsset(entityName);

        return manager->getComponent<T>(id);
    }

};