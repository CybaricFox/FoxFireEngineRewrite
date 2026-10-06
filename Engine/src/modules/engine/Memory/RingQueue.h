//
// Created by cmorg on 9/29/2026.
//

#pragma once
#include "src/defines.h"
#include "foxfire_export.h"
#include "../Library/Logger.h"
#include "src/modules/engine/Memory/FF_Memory.h"

/**
 *  @file RingQueue.h
 *  @layer Engine
 *  @module Memory
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/29/2026
 *
 *  @copyright (c) 2026
 */

template<typename T>
requires std::copy_constructible<T>
class FOXFIRE_API RingQueue {
    static_assert(alignof(T) <= alignof(std::max_align_t),
    "DynamicArray does not support over-aligned types with the current allocator.");

private:
    unsigned int length = 0;
    unsigned int capacity = 0;
    T* memory = nullptr;
    int head = 0;
    int tail = -1;

public:
    RingQueue() = default;
    explicit RingQueue(const unsigned int initialCapacity) {initialize(initialCapacity);}
    ~RingQueue() {shutdown();}

    void initialize(const unsigned int initialCapacity) {
        if (initialCapacity <= 0) {
            Logger::logError("Ring queue requires an initial capacity larger than 0.");
            return;
        }
        if (capacity > 0) {
            Logger::logError("Ring queue is already initialized.");
        }

        capacity = initialCapacity;
        memory = static_cast<T *>(FF_Memory::ff_allocate_storage(sizeof(T) * capacity, RING_QUEUE, alignof(T)));
    }
    void shutdown() {
        if (!memory) return;

        //Empties the queue so all classes are destructed.
        while (length != 0) {
            T temp{};
            dequeue(temp);
        }

        FF_Memory::ff_free_storage(memory, sizeof(T) * capacity, RING_QUEUE, alignof(T));
        capacity = 0;
        length = 0;
        memory = nullptr;
        head = 0;
        tail = -1;
    }

    unsigned int getLength() const {return length;}

    bool enqueue(T* value) {
        if (!value) {
            Logger::logError("Attempted to insert a nullptr into a ring queue.");
            return false;
        }
        if (length == capacity) {
            Logger::logError("Cannot insert. Ring queue is full.");
            return false;
        }

        tail = (tail + 1) % capacity;
        std::construct_at(&memory[tail], *value);
        length++;

        return true;
    }

    bool dequeue(T& outValue) {
        if (length == 0) {
            Logger::logError("Cannot remove. Ring queue is empty.");
            return false;
        }

        outValue = memory[head];
        std::destroy_at(&memory[head]);
        head = (head + 1) % capacity;
        length--;

        return true;
    }

    const T* peek() {
        if (length == 0) {
            Logger::logError("Cannot peek. Ring queue is empty.");
            return nullptr;
        }

        return &memory[head];
    }

};
