//
// Created by cmorg on 11/5/2026.
//

#pragma once
#include "ReusableArray.h"

/**
 *  @file InstanceTracker.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/5/2026
 *
 *  @copyright (c) 2026
 */

/**
 * @brief Allows connecting Objects to UInt ids without having to write an id variable in the object (Very Useful for ECS).
 * @tparam T The object type to track.
 */
template <typename T>
class InstanceTracker {
private:
    ReusableArray<T*> instances{};

public:
    void shutdown() {
        instances.shutdown();
    }
    void shutdownAndFree(const MemoryTag tag) {
        for (unsigned int i = 0; i < instances.getData().getLength(); i++) {
            FF_Memory::ff_free<T>(instances.getData()[i], tag);
        }
        shutdown();
    }

    unsigned int acquireId(T* instance) {
        if (instances.getData().getCapacity() == 0) {
            instances.initialize(100);
            instances.assign();
            instances.getData()[0] = nullptr;
        }

        unsigned int index = instances.assign();
        instances.getData()[index] = instance;

        return index;
    }

    void freeId(unsigned int id) {
        if (instances.getData().getCapacity() == 0) {
            Logger::logError("Attempted to free an id from an instance tracker before the tracker has been initialized.");
            return;
        }

        if (instances.get(id) == nullptr) {
            Logger::logError("Instance Id " + toString(id) + "does not exist.");
            return;
        }

        instances.release(id);
    }

    T* getInstance(unsigned int id) {
        return *instances.get(id);
    }

    unsigned int getInstanceCount() {
       return instances.getCount();
    }
};