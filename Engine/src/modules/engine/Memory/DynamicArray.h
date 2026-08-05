/**
*   @file DynamicArray.h
 *  @layer Engine
 *  @module Memory
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include <utility>
#include <cstddef>

#include "FF_Memory.h"
#include "src/modules/engine/Library/Logger.h"

/**
 * @brief An array designed to be resized. WARNING: Does not update references to internal elements when a resize occurs.
 * @tparam T Type to store in this array
 */
template<typename T>
class FOXFIRE_API DynamicArray {
    static_assert(alignof(T) <= alignof(std::max_align_t),
              "DynamicArray does not support over-aligned types with the current allocator.");
private:
    unsigned long DEFAULT_CAPACITY = 1;
    unsigned long RESIZE_FACTOR = 2;
    MemoryTag tag = DYNAMIC_ARRAY;

    /** @brief the current reserved size of the array */
    unsigned long capacity = 0;
    /** @brief The number of elements in the array. */
    unsigned long length = 0;

    /** @brief The memory the array is using */
    T* memory = nullptr;

    /**
     * @brief Allocated memory space for the array
     * @param size Number of elements in this array
     * @return A pointer to the array.
     */
    T* allocate(const unsigned long size) {
        if (!FF_Memory::isInitialized()) {
            Logger::logFatal("Dynamic Arrays cannot be created prior to FF_Memory!");
            return nullptr;
        }
        return static_cast<T*>(FF_Memory::ff_allocate(sizeof(T) * size, tag));
    }

    void destroy() {
        for (unsigned long i = length; i > 0; --i) {
            std::destroy_at(&memory[i - 1]);
        }
        length = 0;
    }

    void free() {
        if (memory) {
            FF_Memory::ff_free(memory, sizeof(T) * capacity, tag);
            memory = nullptr;
        }
    }

public:
    DynamicArray() = default;
    ~DynamicArray() {shutdown();}

    explicit DynamicArray(const unsigned long initialCapacity) {initialize(initialCapacity);}

    void initialize(const unsigned long newCapacity = 0, const MemoryTag overrideTag = DYNAMIC_ARRAY) {
        if (memory) {
            Logger::logWarn("Dynamic array is already initialized!");
            return;
        }

        unsigned long finalCapacity = newCapacity;
        if (finalCapacity == 0) {
            finalCapacity = DEFAULT_CAPACITY;
        }
        tag = overrideTag;
        memory = allocate(finalCapacity);
        if (!memory) {
            Logger::logFatal(
                "Dynamic Array failed to allocate memory! Size: " +
                std::to_string(sizeof(T)) +
                "B, Capacity: " +
                std::to_string(finalCapacity)
            );
            shutdown();
            return;
        }

        capacity = finalCapacity;
        length = 0;
    }

    DynamicArray(const DynamicArray& other) = delete;
    DynamicArray& operator=(const DynamicArray& other) = delete;

    DynamicArray(DynamicArray&& other) noexcept
        : DEFAULT_CAPACITY(other.DEFAULT_CAPACITY),
    RESIZE_FACTOR(other.RESIZE_FACTOR),
    tag(other.tag),
    capacity(std::exchange(other.capacity, 0)),
    length(std::exchange(other.length, 0)),
    memory(std::exchange(other.memory, nullptr))
    {

    };
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        shutdown();
        DEFAULT_CAPACITY = (other.DEFAULT_CAPACITY);
        RESIZE_FACTOR = (other.RESIZE_FACTOR);
        tag = other.tag;
        capacity = (std::exchange(other.capacity, 0));
        length = (std::exchange(other.length, 0));
        memory = (std::exchange(other.memory, nullptr));

        return *this;
    }

    void shutdown() {
        if (memory) {
            destroy();
            free();
        }

        capacity = 0;
    }

    /** @brief Returns the number of elements the array can currently support. */
    [[nodiscard]] unsigned long getCapacity() const {return capacity;}
    /** @brief Returns the number of elements the array currently has. */
    [[nodiscard]] unsigned long getLength() const {return length;}
    /** @brief Whether the array has any elements */
    [[nodiscard]] bool isEmpty() const {return length == 0;}

    /**
     * @brief Pushes a value to the back of the array.
     * @param value The value to add.
     * @return True on success, false on failure
     */
    bool push(const T& value) {
        if (!memory) {
            Logger::logFatal("Attempted to push a value to a Dynamic Array, but memory is not initialized!");
            return false;;
        }

        if (length >= capacity) {
            if (!resize()) {
                Logger::logFatal("Dynamic Array resizing failed!");
                return false;
            }
        }

        std::construct_at(&memory[length], value);
        length++;
        return true;
    }
    /**
     * @brief Pushes a value to the back of the array.
     * @param value The value to add.
     * @return True on success, false on failure
     */
    bool push(T&& value) {
        if (!memory) {
            Logger::logFatal("Attempted to push a value to a Dynamic Array, but memory is not initialized!");
            return false;
        }

        if (length >= capacity) {
            if (!resize()) {
                Logger::logFatal("Dynamic Array resizing failed!");
                return false;
            }
        }

        std::construct_at(&memory[length], std::move(value));
        length++;
        return true;
    }

    /**
     * @brief Pushes an uninstantiated value to the back of the array.
     * @param args Variables for the struct/constructor
     * @return The constructed element
     */
    template<typename... Args>
    T* emplace(Args&&... args) {
        if (!memory) {
            Logger::logFatal("Attempted to emplace a value to a Dynamic Array, but memory is not initialized!");
            return nullptr;
        }

        if (length >= capacity) {
            if (!resize()) {
                Logger::logFatal("Attempted to emplace a value to a Dynamic Array, but resize failed!");
                return nullptr;
            }
        }

        T* destination = &memory[length];

        if constexpr (std::is_constructible_v<T, Args...>) {
            // Normal constructor path: T(args...)
            std::construct_at(destination, std::forward<Args>(args)...);
        } else if constexpr (requires { T{std::forward<Args>(args)...}; }) {
            // Aggregate/list initialization path: T{args...}
            // Allows fewer members than the struct has.
            std::construct_at(destination, T{std::forward<Args>(args)...});
        } else {
            static_assert(
                std::is_constructible_v<T, Args...>,
                "DynamicArray::emplace failed: T cannot be constructed from these arguments."
            );
        }

        length++;

        return destination;
    }

    /**
     * @brief Removes an element from the array at index. Greater indexes are moved down 1, keeping the array sorted.
     * @param index The index of the element to remove.
     */
    void pop(const unsigned long index) {
        if (index >= length) {
            Logger::logError("Index out of bounds! Length: " + std::to_string(length) + ", Index: " + std::to_string(index));
            return;
        }

        for (unsigned long i = index; i + 1< length; ++i) {
            memory[i] = std::move_if_noexcept(memory[i + 1]);
        }

        length--;
        std::destroy_at(&memory[length]);
    }

    /**
     * @brief Removes an element from the array at index by swapping it with the last element,
     * and then deleting it. Faster than pop() but unsorts the array.
     * @param index The index of the element to remove.
     */
    void unorderedPop(const unsigned long index) {
        if (index >= length) {
            Logger::logError("Index out of bounds! Length: " + std::to_string(length) + ", Index: " + std::to_string(index));
            return;
        }

        const unsigned long lastIndex = length - 1;

        if (index != lastIndex) {
            std::destroy_at(&memory[index]);
            std::construct_at(&memory[index], std::move(memory[lastIndex]));
        }

        std::destroy_at(&memory[lastIndex]);

        length--;
    }

    /**
     * @brief Removes the last element from the array.
     */
    void popBack() {
        if (length < 1) {
            Logger::logError("Attempted to pop an empty Dynamic Array!");
            return;
        }

        length--;
        std::destroy_at(&memory[length]);
    }

    void replace(unsigned long index, T& value) {

    }

    /**
     * @brief Removes all elements in the array.
     */
    void clear() {
        if (!memory) {
            Logger::logWarn("Clear called on Uninitialized Dynamic Array!");
            return;
        }

        destroy();
    }

    /**
     * @brief Inserts a new value to the array at the index. Elements from index and greater are moved up 1.
     * @param value The value to add.
     * @param index The index to add at.
     * @return True on success, False on failure.
     */
    bool insertAt(const T& value, unsigned long index) {
        if (!memory) {
            Logger::logFatal("Attempted to insert into an uninitialized Dynamic Array!");
            return false;
        }

        if (index > length) {
            Logger::logError(
                "Index out of bounds! Length: " + std::to_string(length) +
                ", Index: " + std::to_string(index)
            );
            return false;
        }

        // Protect against inserting an element that already lives inside this array.
        T temp(value);

        if (length >= capacity) {
            if (!resize()) {
                Logger::logFatal("Dynamic Array insertion resizing failed!");
                return false;
            }
        }

        if (index == length) {
            std::construct_at(&memory[length], std::move(temp));
            length++;
            return true;
        }

        // Create a new constructed slot at the end.
        std::construct_at(&memory[length], std::move_if_noexcept(memory[length - 1]));

        // Shift constructed elements backward by assignment.
        for (unsigned long i = length - 1; i > index; i--) {
            memory[i] = std::move_if_noexcept(memory[i - 1]);
        }

        // Replace target slot.
        memory[index] = std::move(temp);

        length++;
        return true;
    }
    /**
     * @brief Inserts a new value to the array at the index. Elements from index and greater are moved up 1.
     * @param value The value to add.
     * @param index The index to add at.
     * @return True on success, False on failure.
     */
    bool insertAt(T&& value, unsigned long index) {
        if (!memory) {
            Logger::logFatal("Attempted to insert into an uninitialized Dynamic Array!");
            return false;
        }

        if (index > length) {
            Logger::logError(
                "Index out of bounds! Length: " + std::to_string(length) +
                ", Index: " + std::to_string(index)
            );
            return false;
        }

        T temp(std::move(value));

        if (length >= capacity) {
            if (!resize()) {
                Logger::logFatal("Dynamic Array insertion resizing failed!");
                return false;
            }
        }

        if (index == length) {
            std::construct_at(&memory[length], std::move(temp));
            length++;
            return true;
        }

        std::construct_at(&memory[length], std::move_if_noexcept(memory[length - 1]));

        for (unsigned long i = length - 1; i > index; i--) {
            memory[i] = std::move_if_noexcept(memory[i - 1]);
        }

        memory[index] = std::move(temp);

        length++;
        return true;
    }

    /**
     * @brief Sets the capacity of the array to the give capacity.
     * @param newCapacity Capacity to reserve.
     */
    void ensureSize(const unsigned long newCapacity) {
        if (newCapacity > capacity) {
            if (memory) {
                resize(newCapacity);
            }
        }
    }

    /**
     * @brief Creates a copy of an element.
     * @return Returns the copy.
     */
    T* duplicate() {
        Logger::logWarn("Duplicate is not fully setup and may cause unintended behavior!");

        if (!memory || capacity == 0) {
            return nullptr;
        }

        T* copy = allocate(capacity);

        if (!copy) {
            Logger::logFatal("Dynamic Array duplicate failed to allocate memory!");
            return nullptr;
        }

        unsigned long constructed = 0;

        try {
            for (; constructed < length; constructed++) {
                std::construct_at(&copy[constructed], memory[constructed]);
            }
        } catch (...) {
            for (unsigned long i = constructed; i > 0; i--) {
                std::destroy_at(&copy[i - 1]);
            }

            FF_Memory::ff_free(copy, sizeof(T) * capacity, tag);
            Logger::logFatal("Dynamic Array duplicate failed while copying elements!");
            return nullptr;
        }

        return copy;
    }

    T* getData() {
        return memory;
    }

private:
    /**
     * @brief Resizes the array. If requiredSize is given, Resizes it to that size instead.
     * @param requiredSize Optional size to resize to.
     * @return True on success, False on failure.
     */
    bool resize(unsigned long requiredSize = 0) {
        if (!memory || capacity == 0) {
            Logger::logFatal("Dynamic Array resize called on an array of size 0!");
            return false;
        }

        if (requiredSize <= capacity) {
            requiredSize = capacity * RESIZE_FACTOR;
        }

        T* temp = allocate(requiredSize);

        if (!temp) {
            Logger::logFatal("Dynamic Array resize failed to allocate memory!");
            return false;
        }

        unsigned long i = 0;
        try {
            for (; i < length; ++i) {
                std::construct_at(&temp[i], std::move_if_noexcept(memory[i]));
            }
        } catch (...) {
            for (unsigned long j = i; j > 0; --j) {
                std::destroy_at(&temp[j - 1]);
            }

            FF_Memory::ff_free(temp, sizeof(T) * requiredSize, tag);
            Logger::logFatal("Dynamic Array reallocate failed while moving elements!");
            return false;
        }

        destroy();
        free();

        memory = temp;
        capacity = requiredSize;
        length = i;

        return true;
    };

public:
    T& operator[](unsigned long index) {
        return memory[index];
    }
    const T& operator[](unsigned long index) const {
        return memory[index];
    }

    //FOR LOOP COMPATIBILITY
    T* begin() {
        return memory;
    }

    T* end() {
        return memory + length;
    }

    const T* begin() const {
        return memory;
    }

    const T* end() const {
        return memory + length;
    }

    const T* cbegin() const {
        return memory;
    }

    const T* cend() const {
        return memory + length;
    }
    //COMPATIBILITY END
};
