/**
*   @file Contexts.h
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
#include "src/defines.h"

struct AssetContext {
    /** @brief number of references to this asset. */
    unsigned long referenceCount = 0;
    /** @brief index of this asset in the asset map. */
    unsigned int index = INVALID_ID_U32;
    /** @brief Whether to remove this asset when there are no references. */
    bool bAutoRelease = false;
};
