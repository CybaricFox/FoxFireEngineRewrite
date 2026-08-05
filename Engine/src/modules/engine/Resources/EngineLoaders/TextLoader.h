/**
*   @file TextLoader.h
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
#include "src/modules/engine/Resources/ResourceLoader.h"

/**
 * @brief Loads txt Files.
 */
class TextLoader final : public ResourceLoader{
public:
    TextLoader();

    bool load(String name, Resource &outResource, String basePath) override;
};
