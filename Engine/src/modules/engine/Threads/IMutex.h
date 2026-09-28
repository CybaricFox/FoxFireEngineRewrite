//
// Created by cmorg on 9/28/2026.
//

#pragma once

/**
 *  @file IMutex.h
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
class IMutex {
private:
    void* data = nullptr;

public:
    virtual ~IMutex() = default;

    virtual bool createMutex() = 0;
    virtual void destroyMutex() = 0;

    /**
     * @brief Locks the mutex, preventing other threads from taking the resource.
     * @return True if lock was successful.
     */
    virtual bool lock() = 0;

    /**
     * @brief Unlocks the mutex, allowing other threads to take the resource.
     * @return True if the lock was removed.
     */
    virtual bool unlock() = 0;
};