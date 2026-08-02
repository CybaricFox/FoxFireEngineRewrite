//
// Created by cmorg on 8/2/2026.
//

#pragma once
#include "src/modules/engine/Memory/DynamicArray.h"

//A type of dynamic array that does not delete values when they are no longer in use.
//Values are kept for reuse later on. Release frees the index. Assign checks for freed indexes first and then emplaces a new value if no indexes are free.
template<typename T>
class ReusableArray {
private:
    DynamicArray<T> data{};
    DynamicArray<unsigned int> freeIndexes{};

public:
    void initialize(unsigned int initialCapacity) {
        data.initialize(initialCapacity, REUSABLE_ARRAY);
        freeIndexes.initialize(0, REUSABLE_ARRAY);
    }
    void shutdown() {
        freeIndexes.shutdown();
        data.shutdown();
    }

    unsigned int assign() {
        unsigned int index = INVALID_ID;

        if (freeIndexes.isEmpty()) {
            index = data.getLength();
            data.emplace();
        } else {
            index = freeIndexes[freeIndexes.getLength() - 1];
            freeIndexes.popBack();
        }

        return index;
    }

    void release(const unsigned int index) {
        freeIndexes.push(index);
    }

    T& get(unsigned int index) {
        return data[index];
    }

    DynamicArray<T>& getData() {return data;}
};
