//
// Created by cmorg on 8/3/2026.
//

#include "ResourceSystem.h"

#include "EngineLoaders/BinaryLoader.h"
#include "EngineLoaders/BitmapFontLoader.h"
#include "EngineLoaders/ImageLoader.h"
#include "EngineLoaders/MaterialLoader.h"
#include "EngineLoaders/MeshLoader.h"
#include "EngineLoaders/ShaderLoader.h"
#include "EngineLoaders/SystemFontLoader.h"
#include "EngineLoaders/TextLoader.h"

ResourceSystem* ResourceSystem::instance = nullptr;

bool ResourceSystem::initialize(const String &path, const unsigned int initialCapacity) {
    instance = this;

    loaders.initialize(initialCapacity);
    assetsPath = path;

    //Register engine loaders
    registerLoader(FF_Memory::ff_allocate<ImageLoader>(RESOURCE));
    registerLoader(FF_Memory::ff_allocate<MaterialLoader>(RESOURCE));
    registerLoader(FF_Memory::ff_allocate<BinaryLoader>(RESOURCE));
    registerLoader(FF_Memory::ff_allocate<TextLoader>( RESOURCE));
    registerLoader(FF_Memory::ff_allocate<ShaderLoader>(RESOURCE));
    registerLoader(FF_Memory::ff_allocate<MeshLoader>(RESOURCE));
    registerLoader(FF_Memory::ff_allocate<BitmapFontLoader>(RESOURCE));
    registerLoader(FF_Memory::ff_allocate<SystemFontLoader>(RESOURCE));

    Logger::logInfo("Resource system initialized with path: " + assetsPath);
    return true;
}

void ResourceSystem::shutdown() {
    for (IResourceLoader* loader : loaders) {
        FF_Memory::ff_free<IResourceLoader>(loader, RESOURCE, loader->getMemorySize());
    }
    loaders.shutdown();

    assetsPath.destroy();
}

bool ResourceSystem::registerLoader(IResourceLoader* loader) {
    for (const IResourceLoader* l : loaders) {
        if (l->getType() == loader->getType()) {
            Logger::logError("Tried to register a resource loader but an identical resource loader already exists! Type id: " + toString(loader->getType()));
            return false;
        }
        if (loader->isCustomType() && l->getCustomType() == loader->getCustomType()) {
            Logger::logError("Tried to register a custom resource loader but an identical one already exists! Type name: " + loader->getCustomType());
            return false;
        }
    }

    loader->setId(loaders.getLength());
    loaders.push(loader);
    Logger::logDebug("Loader registered successfully at: " + toString(loader->getId()));
    return true;
}

bool ResourceSystem::load(const String &name, const ResourceType type, Resource &outResource, ILoaderParameters* params) {
    if (type == RESOURCE_TYPE_CUSTOM) {
        outResource.loaderId = INVALID_ID_U32;
        Logger::logError("Load called for a custom type! Did you mean to call loadCustom?");
        return false;
    }

    for (IResourceLoader* loader : instance->loaders) {
        if (loader->getId() != INVALID_ID_U32 && loader->getType() == type) {
            outResource.loaderId = loader->getId();
            return loader->load(name, outResource, instance->assetsPath, params);
        }
    }

    Logger::logError("Cannot find a resource loader for type: "+ toString(type));
    return false;
}

bool ResourceSystem::loadCustom(const String &name, const String &type, Resource &outResource, ILoaderParameters* params) {
    if (type.isEmpty()) {
        outResource.loaderId = INVALID_ID_U32;
        Logger::logError("LoadCustom called with an empty name!");
        return false;
    }

    for (IResourceLoader* loader : loaders) {
        if (loader->getId() != INVALID_ID_U32 && loader->getType() == RESOURCE_TYPE_CUSTOM && name.equalsIgnoreCase(loader->getCustomType())) {
            outResource.loaderId = loader->getId();
            return loader->load(name, outResource, assetsPath, params);
        }
    }

    Logger::logError("Cannot find a custom resource loader for type: "+ type);
    return false;
}

void ResourceSystem::unload(Resource &resource) {
    if (resource.loaderId == INVALID_ID_U32) return;

    IResourceLoader& loader = *instance->loaders[resource.loaderId];
    if (loader.getId() == INVALID_ID_U32) return;

    loader.unload(resource);
}
