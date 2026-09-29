//
// Created by cmorg on 9/26/2026.
//

#pragma once
#include "src/defines.h"

/**
 *  @file Thread.h
 *  @layer Engine
 *  @module Threads
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/26/2026
 *
 *  @copyright (c) 2026
 */

/**
 * @brief A function used by threads. Returns a Uint and takes a void* as a param.
 */
using ThreadFunction = unsigned int(*)(void*);
/**
 * @brief A function used by threads. Works identically to ThreadFunction but also includes an outResult argument.
 */
using ResultFunction = bool(*)(void*, void*&);

struct Thread {
    ULong id = INVALID_ID_U64;
    ThreadFunction threadFunction = nullptr;
    void* data = nullptr;
};