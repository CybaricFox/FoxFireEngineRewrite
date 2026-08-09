//
// Created by cmorg on 8/7/2026.
//

#pragma once
#include "src/modules/engine/Resources/ResourceLoader.h"

/**
 *  @file ShaderLoader.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/7/2026
 *
 *  @copyright (c) 2026
 */

class ShaderLoader : public ResourceLoader{
public:
    ShaderLoader();

    bool load(String name, Resource &outResource, String basePath) override;
    void unload(Resource &resource) override;
};