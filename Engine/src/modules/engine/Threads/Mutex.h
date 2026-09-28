//
// Created by cmorg on 9/28/2026.
//

#pragma once

/**
 *  @file Mutex.h
 *  @layer Engine
 *  @module Threads
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/28/2026
 *
 *  @copyright (c) 2026
 */

/**
 * @brief Limits access to a resource that is multithreaded.
 */
struct Mutex {
    void* data = nullptr;
};