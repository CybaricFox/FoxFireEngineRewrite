//
// Created by cmorg on 8/3/2026.
//

#pragma once
#include "src/modules/engine/Resources/ResourceLoader.h"


class MaterialLoader final : public ResourceLoader{
public:
    MaterialLoader();

    bool load(String name, Resource &outResource, String basePath) override;
    void unload(Resource &resource) override;
};
