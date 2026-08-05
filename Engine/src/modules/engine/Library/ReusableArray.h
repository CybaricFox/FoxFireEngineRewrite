/**
*   @file ReusableArray.h
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
#include "src/modules/engine/Memory/DynamicArray.h"

//A type of dynamic array that does not delete values when they are no longer in use.
//Values are kept for reuse later on. Release frees the index. Assign checks for freed indexes first and then emplaces a new value if no indexes are free.
/**
 * @brief A dynamic array that reuses cleared elements instead of freeing them.
 * Cleared elements are priorized over creating new elements when available.
 * @tparam T Type of the element to store
 */
template<typename T>
class ReusableArray {
private:
    /** @brief stores the type */
    DynamicArray<T> data{};
    /** @brief array of indexes that were previously cleared. */
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

    /**
     * @brief Assigns an index to be used. Prioritizes cleared indexes over creating new ones.
     * @return The index that is now available for use.
     */
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

    /**
     * @brief Clears an index
     * @param index The index to clear
     */
    void release(const unsigned int index) {
        freeIndexes.push(index);
    }

    /**
     * @brief Gets the element stored at an index
     * @param index The index of the element
     * @return Reference to the element at that index
     */
    T& get(unsigned int index) {
        return data[index];
    }

    /**
     * @brief Fetches the DynamicArray used to store the elements. Meant for use in for loops.
     * @return Reference to the dynamic array that stores the elements.
     */
    DynamicArray<T>& getData() {return data;}
};
