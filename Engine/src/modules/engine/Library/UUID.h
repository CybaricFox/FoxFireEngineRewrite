//
// Created by cmorg on 11/5/2026.
//

#pragma once
#include "String.h"

/**
 *  @file UUID.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/5/2026
 *
 *  @copyright (c) 2026
 */

struct UUID {
    String id{};
};

class UUIDUtils {
public:
    static void setSeed(ULong seed);

    /**
     * @brief Generates a UUID using the seed. WARNING: CURRENT IMPLEMENTATION DOES NOT USE A SEED AND IS PRONE TO DUPLICATES!!!
     * @return
     */
    static UUID generate();
};