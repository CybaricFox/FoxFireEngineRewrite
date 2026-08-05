/**
*   @file HashUtils.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "src/defines.h"
#include <foxfire_export.h>

/**
 * @brief Collection of functions for hashing types.
 */
class FOXFIRE_API HashUtils {
private:
    /** @brief multiplier to be added to the hash. Must be a prime number */
    static unsigned long multiplier;

public:
    static unsigned long generateStringHash(const String& key);
};
