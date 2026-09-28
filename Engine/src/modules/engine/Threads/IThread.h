//
// Created by cmorg on 9/26/2026.
//

#pragma once
#include "src/defines.h"

/**
 *  @file IThread.h
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

class IThread {
private:
    ULong id = INVALID_ID_U64;
    ThreadFunction threadFunction = nullptr;

public:
    virtual ~IThread() = default;

    /**
     * @brief Creates a thread
     * @param newFunction The function that will be called immediately
     * @param params The parameters for that function.
     * @param autoDetach If true, the thread will be shutdown when finished.
     * @return False if something goes wrong.
     */
    virtual bool createThread(ThreadFunction newFunction, void* params, bool autoDetach) = 0;
    virtual void destroyThread() = 0;

    [[nodiscard]] virtual bool isActive() const = 0;
    [[nodiscard]] ULong getId() const {return id;}

    /**
     * @brief Detaches this thread from the main thread so it isn't waiting on this to finish.
     */
    virtual void detach() = 0;

    /**
     * @brief Attempts to cancel the work on this thread.
     */
    virtual void cancel() = 0;
    virtual void sleep(ULong ms) = 0;
};