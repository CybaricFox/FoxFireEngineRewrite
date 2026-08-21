//
// Created by cmorg on 8/12/2026.
//

#include "EntityManager.h"


EntityManager::EntityManager() {
    memorySize = DynamicAllocator::getMemoryRequirement(MEBIBYTES(1));
    memory = FF_Memory::ff_allocate(memorySize, ECS);
    allocator.initialize(MEBIBYTES(1), memory);
    instances.initialize();
}

EntityManager::~EntityManager() {
    instances.shutdown();
    allocator.shutdown();
    FF_Memory::ff_free(memory, memorySize, ECS);
}

void EntityManager::copyFromTemplate(Entity &entity, const unsigned int id) {
    unsigned long totalSize = sizeof(EntityInstance);
    for (const EntityComponent* component : entity.components) {
        totalSize += component->componentSize;
    }

    void* location = allocator.allocate(totalSize);

    EntityInstance* instance = std::construct_at(static_cast<EntityInstance *>(location));
    instance->id = id;
    instance->totalSize = totalSize;

    //Push the memory location to the array for lookup later
    instances.push(instance);

    location = static_cast<unsigned char *>(location) + sizeof(EntityInstance);

    unsigned int newCount = 0;
    for (const EntityComponent* component : entity.components) {
        FF_Memory::ff_copy(location, component, component->componentSize);
        location = static_cast<unsigned char *>(location) + component->componentSize;
        newCount++;
    }

    instance->componentCount = newCount;

    count++;
}
