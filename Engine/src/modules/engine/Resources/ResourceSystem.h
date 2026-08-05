/**
*   @file ResourceSystem.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "ResourceLoader.h"
#include "src/defines.h"
#include "src/modules/engine/Memory/DynamicArray.h"

/**
 * @brief Controls resource loading and management
 */
class ResourceSystem {
private:
    /** @brief The relative path to the Assets folder */
    String assetsPath;
    /** @brief An array of loaders */
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
