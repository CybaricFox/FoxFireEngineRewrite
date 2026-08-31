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
    for (EntityInstance* instance : instances) {
        if (instance->componentCount == 0) continue;

        auto component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(instance) + sizeof(EntityInstance));
        for (unsigned int i = 1; i <= instance->componentCount; i++) {
            const unsigned long size = component->getComponentSize();
            std::destroy_at(component);
            component = reinterpret_cast<EntityComponent *>(reinterpret_cast<unsigned char *>(component) + size);
        }

    }
    instances.shutdown();

    allocator.shutdown();
    FF_Memory::ff_free(memory, memorySize, ECS);
}

void EntityManager::copyFromTemplate(Entity &entity, const unsigned int id) {
    unsigned long totalSize = sizeof(EntityInstance);
    for (EntityComponent* component : entity.components) {
        totalSize += component->getComponentSize();
    }

    void* location = allocator.allocate(totalSize);

    EntityInstance* instance = std::construct_at(static_cast<EntityInstance *>(location));
    instance->id = id;
    instance->totalSize = totalSize;

    //Push the memory location to the array for lookup later
    instances.push(instance);

    location = static_cast<unsigned char *>(location) + sizeof(EntityInstance);

    unsigned int newCount = 0;
    for (EntityComponent* component : entity.components) {
        component->copyTo(static_cast<EntityComponent *>(location));
        location = static_cast<unsigned char *>(location) + component->getComponentSize();
        newCount++;
    }

    instance->componentCount = newCount;

    count++;
}
