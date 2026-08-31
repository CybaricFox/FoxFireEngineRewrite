//
// Created by cmorg on 8/3/2026.
//

#include "ResourceSystem.h"

#include "EngineLoaders/BinaryLoader.h"
#include "EngineLoaders/ImageLoader.h"
#include "EngineLoaders/MaterialLoader.h"
#include "EngineLoaders/MeshLoader.h"
#include "EngineLoaders/ShaderLoader.h"
#include "EngineLoaders/TextLoader.h"
#include "src/modules/engine/Library/StringUtils.h"

bool ResourceSystem::initialize(const String &path, const unsigned int initialCapacity) {
    loaders.initialize(initialCapacity);
    assetsPath = path;

    //Register engine loaders
    registerLoader(FF_Memory::ff_allocate_class<ImageLoader>(sizeof(ImageLoader), RESOURCE));
    registerLoader(FF_Memory::ff_allocate_class<MaterialLoader>(sizeof(MaterialLoader), RESOURCE));
    registerLoader(FF_Memory::ff_allocate_class<BinaryLoader>(sizeof(BinaryLoader), RESOURCE));
    registerLoader(FF_Memory::ff_allocate_class<TextLoader>(sizeof(TextLoader), RESOURCE));
    registerLoader(FF_Memory::ff_allocate_class<ShaderLoader>(sizeof(ShaderLoader), RESOURCE));
    registerLoader(FF_Memory::ff_allocate_class<MeshLoader>(sizeof(MeshLoader), RESOURCE));

    Logger::logInfo("Resource system initialized with path: " + assetsPath);
    return true;
}

void ResourceSystem::shutdown() {
    for (ResourceLoader* loader : loaders) {
        FF_Memory::ff_free_class<ResourceLoader>(loader, loader->getMemorySize(), RESOURCE);
    }
    loaders.shutdown();
}

bool ResourceSystem::registerLoader(ResourceLoader* loader) {
    for (const ResourceLoader* l : loaders) {
        if (l->getType() == loader->getType()) {
            Logger::logError("Tried to register a resource loader but an identical resource loader already exists! Type id: " + std::to_string(loader->getType()));
            return false;
        }
        if (loader->isCustomType() && l->getCustomType() == loader->getCustomType()) {
            Logger::logError("Tried to register a custom resource loader but an identical one already exists! Type name: " + loader->getCustomType());
            return false;
        }
    }

    loader->setId(loaders.getLength());
    loaders.push(loader);
    Logger::logDebug("Loader registered successfully at: " + std::to_string(loader->getId()));
    return true;
}

bool ResourceSystem::load(const String &name, const ResourceType type, Resource &outResource) {
    if (type == RESOURCE_TYPE_CUSTOM) {
        outResource.loaderId = INVALID_ID_U32;
        Logger::logError("Load called for a custom type! Did you mean to call loadCustom?");
        return false;
    }

    for (ResourceLoader* loader : loaders) {
        if (loader->getId() != INVALID_ID_U32 && loader->getType() == type) {
            outResource.loaderId = loader->getId();
            return loader->load(name, outResource, assetsPath);
        }
    }

    Logger::logError("Cannot find a resource loader for type: "+ std::to_string(type));
    return false;
}

bool ResourceSystem::loadCustom(const String &name, const String &type, Resource &outResource) {
    if (type.empty()) {
        outResource.loaderId = INVALID_ID_U32;
        Logger::logError("LoadCustom called with an empty name!");
        return false;
    }

    for (ResourceLoader* loader : loaders) {
        if (loader->getId() != INVALID_ID_U32 && loader->getType() == RESOURCE_TYPE_CUSTOM && StringUtils::equalsIgnoreCase(name, loader->getCustomType())) {
            outResource.loaderId = loader->getId();
            return loader->load(name, outResource, assetsPath);
        }
    }

    Logger::logError("Cannot find a custom resource loader for type: "+ type);
    return false;
}

void ResourceSystem::unload(Resource &resource) {
    if (resource.loaderId == INVALID_ID_U32) return;

    ResourceLoader& loader = *loaders[resource.loaderId];
    if (loader.getId() == INVALID_ID_U32) return;

    loader.unload(resource);
}
