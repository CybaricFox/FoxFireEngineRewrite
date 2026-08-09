/**
*   @file ResourceLoader.h
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
#include "EngineResourceTypes.h"
#include "src/defines.h"
#include "src/modules/engine/Library/Logger.h"

/**
 * @brief Abstract Class used to load various resources.
 */
class ResourceLoader {
protected:
    /** @brief the id of the loader */
    unsigned int id = INVALID_ID_U32;
    /** @brief the type of resource to load */
    ResourceType type{};
    /** @brief the type of custom resource to load */
    String customType{};
    /** @brief The specific path of the resource directory starting from Assets/ */
    String path{};
    /** @brief The memory tag to use for allocation tracking */
    MemoryTag memoryTag = UNKNOWN;

public:
    virtual ~ResourceLoader() = default;

    ResourceType getType() const {return type;}
    String getCustomType() const {return customType;}
    unsigned int getId() const {return id;}

    bool isCustomType() const {return !customType.empty();}
    void setId(const unsigned int newId) {
        if (id == INVALID_ID_U32) {
            id = newId;
        } else {
            Logger::logError("Resource system cannot assign an id to a loader because the loader already has an id! New ID: " + std::to_string(newId) + " Existing Id: " + std::to_string(id));
        }
    }

    virtual bool load(String name, Resource& outResource, String basePath) = 0;
    virtual void unload(Resource &resource);

};
