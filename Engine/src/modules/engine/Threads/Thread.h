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

using ThreadFunction = void(*)(void*);

struct Thread {
    ULong id = INVALID_ID_U64;
    ThreadFunction threadFunction = nullptr;
    void* data = nullptr;
};