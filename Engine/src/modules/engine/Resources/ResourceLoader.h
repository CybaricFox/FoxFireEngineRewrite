//
// Created by cmorg on 8/3/2026.
//

#pragma once
#include "EngineResourceTypes.h"
#include "src/defines.h"
#include "src/modules/engine/Library/Logger.h"


class ResourceLoader {
protected:
    unsigned int id = INVALID_ID;
    ResourceType type{};
    String customType{};
    String path{};

public:
    virtual ~ResourceLoader() = default;

    ResourceType getType() const {return type;}
    String getCustomType() const {return customType;}
    unsigned int getId() const {return id;}

    bool isCustomType() const {return !customType.empty();}
    void setId(const unsigned int newId) {
        if (id == INVALID_ID) {
            id = newId;
        } else {
            Logger::logError("Resource system cannot assign an id to a loader because the loader already has an id! New ID: " + std::to_string(newId) + " Existing Id: " + std::to_string(id));
        }
    }

    virtual bool load(String name, Resource& outResource, String basePath) = 0;
    virtual void unload(Resource& resource) = 0;

};
