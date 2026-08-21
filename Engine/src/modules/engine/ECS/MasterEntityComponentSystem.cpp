//
// Created by cmorg on 8/12/2026.
//

#include "MasterEntityComponentSystem.h"

#include "Engine_Components/Mesh.h"
#include "Engine_Components/Transform.h"

String MasterEntityComponentSystem::getEntityName(const unsigned int id) {
    if (entities.isEmpty()) return "";
    unsigned int minIndex = 0;
    unsigned int maxIndex = entities.getLength() - 1;

    while (minIndex < maxIndex) {
        const unsigned int targetIndex = minIndex + ((maxIndex - minIndex) / 2);
        const unsigned int targetId = entities[targetIndex].id;
        if (targetId == id) {
            return entities[targetIndex].entityName;
        }

        if (targetId < id) {
            minIndex = targetIndex + 1;
        } else {
            maxIndex = targetIndex;
        }
    }

    //If min and max are the same, check them before throwing.
    if (entities[minIndex].id == id) return entities[minIndex].entityName;

    Logger::logWarn("Failed to retrieve name for entity: " + std::to_string(id) + " Entity does not exist!");
    return "";
}

Entity *MasterEntityComponentSystem::createEntityType(const String &name) {
    AssetContext context{};
    Entity* entity = templates.createAsset(name, context);
    context.bAutoRelease = false;

    return entity;
}

void MasterEntityComponentSystem::initialize() {
    templates.initialize(0);
    entities.initialize(0);

    Entity* basic = createEntityType("Basic_Entity");

    basic->components.initialize(0, ECS);
    Transform* transform = FF_Memory::ff_allocate_class<Transform>(sizeof(Transform), ECS);
    basic->components.push(transform);
    Mesh* mesh = FF_Memory::ff_allocate_class<Mesh>(sizeof(Mesh), ECS);
    basic->components.push(mesh);

    instances.initialize(templates.getAssetCount());
}

void MasterEntityComponentSystem::shutdown() {
    entities.shutdown();

    for (Entity* entity : templates.getAssetsAsArray()) {
        for (EntityComponent* component : entity->components) {
            FF_Memory::ff_free_class<EntityComponent>(component, component->componentSize, ECS);
        }
        entity->components.shutdown();
    }
    templates.shutdown();
    instances.shutdown();
}

unsigned int MasterEntityComponentSystem::getEntityCount(const String &name) {
    const EntityManager* manager = instances.getAsset(name);

    return manager->getEntityCount();
}

unsigned int MasterEntityComponentSystem::createEntity(const String &name) {
    Entity* entity = templates.getAsset(name);
    EntityManager* manager = instances.getAsset(name);

    if (manager == nullptr) {
        AssetContext context{};
        manager = instances.createAsset(name, context);
        context.bAutoRelease = false;
    }

    const unsigned int id = getNewId();
    manager->copyFromTemplate(*entity, id);
    entities.push({id, name});

    return id;
}
