//
// Created by cmorg on 8/3/2026.
//

#pragma once
#include "ResourceLoader.h"
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"

class ResourceSystem {
private:
    //Relative path to assets folder
    String assetsPath;
    DynamicArray<ResourceLoader*> loaders{};

    bool registerLoader(ResourceLoader *loader);

public:
    bool initialize(const String &path, unsigned int initialCapacity);
    void shutdown();

    String getAssetPath() {return assetsPath;}

    bool load(const String &name, ResourceType type, Resource& outResource);
    bool loadCustom(const String &name, const String &type, Resource& outResource);
    void unload(Resource& resource);
};
