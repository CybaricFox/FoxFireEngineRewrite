//
// Created by cmorg on 11/19/2026.
//

#pragma once
#include "src/modules/engine/Memory/DynamicArray.h"

template <typename T>
class Stack {
private:
    DynamicArray<T*> stack{};

public:
    ~Stack() {shutdown();}
    void initialize() {stack.initialize(0, DYNAMIC_ARRAY);}
    void shutdown() {stack.shutdown();}

    unsigned int getSize() {return stack.getLength();}
    DynamicArray<T*>& getData() {return stack;}

    bool push(T* object) {
        if (object == nullptr) {
            Logger::logError("Cannot push a null object onto a stack!");
            return false;
        }

        stack.push(object);
        return true;
    }

    /**
     * @brief Gets the object at the top of the stack and removes it.
     * @return
     */
    T* pull() {
        if (stack.getLength() == 0) {
            Logger::logError("Cannot pull from an empty stack!");
            return nullptr;
        }

        T* result = stack[stack.getLength() - 1];

        if (result == nullptr) {
            Logger::logFatal("WARNING!!! STACK RESULT IS NULL!!! POSSIBLE MEMORY CORRUPTION!!!");
        }

        stack.popBack();
        return result;
    }

    /**
     * @brief Gets the object at the top of the stack. Does not remove it from the stack
     * @return
     */
    T* const& get() {
        if (stack.getLength() == 0) {
            Logger::logError("Cannot get from an empty stack!");
            return nullptr;
        }

        T* const& result = stack[stack.getLength() - 1];

        if (result == nullptr) {
            Logger::logFatal("WARNING!!! STACK RESULT IS NULL!!! POSSIBLE MEMORY CORRUPTION!!!");
        }

        return result;
    }
};
