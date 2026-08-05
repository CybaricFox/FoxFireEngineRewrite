/**
*   @file ImageLoader.h
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
 * @brief Loads Image Files such as .png
 */
class ImageLoader final : public ResourceLoader{
public:
    ImageLoader();

    bool load(String name, Resource &outResource, String basePath) override;
};
