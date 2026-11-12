//
// Created by cmorg on 8/12/2026.
//

#include "EntityComponentSystem.h"

#include "Engine_Components/Camera.h"
#include "Engine_Components/Mesh.h"
#include "Engine_Components/Transform.h"

EntityComponentSystem* EntityComponentSystem::instance = nullptr;

String EntityComponentSystem::getEntityName(const unsigned int id) {
    const EntityContext* context = instance->entities.getInstance(id);
    if (!context) {
        Logger::logWarn("Failed to retrieve name for entity: " + toString(id) + " Entity does not exist!");
        return {};
    }

    return context->entityName;
}

Entity *EntityComponentSystem::createEntityType(const String &name) {
    return templates.createAsset(name, false);
}

void EntityComponentSystem::initialize() {
    instance = this;

    templates.initialize(0);

    Entity* basic = createEntityType("Basic_Entity");
    basic->components.initialize(0, ECS);
    const auto transform = FF_Memory::ff_allocate<Transform>(ECS);
    basic->components.push(transform);
    const auto mesh = FF_Memory::ff_allocate<Mesh>(ECS);
    basic->components.push(mesh);

    Entity* basicUI = createEntityType("Basic_UI");
    basicUI->components.initialize(0, ECS);
    const auto transformUI = FF_Memory::ff_allocate<Transform>(ECS);
    basicUI->components.push(transformUI);
    const auto meshUI = FF_Memory::ff_allocate<Mesh>(ECS);
    basicUI->components.push(meshUI);

    instances.initialize(1024);
}

void EntityComponentSystem::shutdown() {
    entities.shutdownAndFree(ECS);

    for (Entity* entity : templates.getAssetsAsArray()) {
        for (EntityComponent* component : entity->components) {
            FF_Memory::ff_free<EntityComponent>(component, ECS, component->getComponentSize());
        }
        entity->components.shutdown();
    }

    templates.shutdown();
    instances.shutdown();

    instance = nullptr;
}

unsigned int EntityComponentSystem::getEntityCount(const String &name) {
    const EntityManager* manager = instances.getAsset(name);

    return manager->getEntityCount();
}

DynamicArray<unsigned int>& EntityComponentSystem::getAllEntitiesOfType(const String &type) {
    EntityManager* manager = instance->instances.getAsset(type);
    const unsigned int count = manager->getEntityCount();
    auto& result = *FF_Memory::ff_allocate<DynamicArray<unsigned int>>(DYNAMIC_ARRAY);
    result.initialize(count);

    for (EntityInstance*& entity : manager->getInstances()) {
        result.push(entity->id);
    }

    return result;
}

unsigned int EntityComponentSystem::createEntity(const String &name) {
    Entity* entity = templates.getAsset(name);
    EntityManager* manager = instances.getAsset(name);

    if (manager == nullptr) {
        manager = instances.createAsset(name, false);
    }

    EntityContext& context = *FF_Memory::ff_allocate<EntityContext>(ECS);
    const unsigned int id = getNewId(context);
    context.id = id;
    context.entityName = name;
    manager->copyFromTemplate(*entity, id);

    return id;
}
