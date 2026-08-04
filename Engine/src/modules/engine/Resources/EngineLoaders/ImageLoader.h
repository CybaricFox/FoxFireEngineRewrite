//
// Created by cmorg on 8/3/2026.
//

#pragma once
#include "src/modules/engine/Resources/ResourceLoader.h"

class ImageLoader final : public ResourceLoader{
public:
    ImageLoader();

    bool load(String name, Resource &outResource, String basePath) override;
};
