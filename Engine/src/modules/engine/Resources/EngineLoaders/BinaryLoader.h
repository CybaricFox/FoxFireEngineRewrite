/**
*   @file BinaryLoader.h
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
#include "src/modules/engine/Resources/IResourceLoader.h"

/**
 * @brief Loads binary files.
 */
class BinaryLoader final : public IResourceLoader{
public:
    BinaryLoader();

    bool load(String name, Resource &outResource, String basePath, ILoaderParameters *params) override;
};
