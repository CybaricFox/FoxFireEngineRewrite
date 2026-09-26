//
// Created by cmorg on 9/26/2026.
//

#pragma once
#include "src/modules/engine/Input/IInputSystem.h"

struct KeyState {
    Buttons button;
    Keys key;
    bool bIsPressed;
};

struct MouseState {
    short x;
    short y;
    char z;
};

class PlatformState {
private:
    ULong size = 0;

protected:
    DynamicArray<KeyState> keyInputs{};
    DynamicArray<MouseState> mouseInputs{};

public:
    explicit PlatformState(const ULong newSize) : size(newSize) {};
    virtual ~PlatformState() = default;

    ULong getSize() const { return size; }

    void processInputs(IInputSystem& inputSystem) {
        for (const KeyState& input : keyInputs) {
            if (input.key == MAX_KEYS) {
                inputSystem.processButton(input.button, input.bIsPressed);
            } else {
                inputSystem.processKey(input.key, input.bIsPressed);
            }
        }
        keyInputs.clear();

        for (const MouseState& mouse : mouseInputs) {
            if (mouse.z != 0) {
                inputSystem.processMouseScroll(mouse.z);
            } else {
                inputSystem.processMouseMove(mouse.x, mouse.y);
            }
        }
        mouseInputs.clear();
    }

    virtual bool initialize(const String &applicationName, const int x, const int y, const int width, const int height) {
        keyInputs.initialize();
        mouseInputs.initialize();

        return true;
    }
    virtual void shutdown() {
        keyInputs.shutdown();
        mouseInputs.shutdown();
    }

    /**
     * @brief Prints a message to console.
     * @param message The message to print
     * @param color Color of the message.
     */
    virtual void printConsoleMessage(const String& message, unsigned char color) = 0;

    /**
     * @brief Prints a message to console via the error stream.
     * @param message The message to print.
     * @param color The color of the message.
     */
    virtual void printConsoleError(const String& message, unsigned char color) = 0;

    virtual bool createSurface() = 0;
    virtual double getAbsoluteTime() = 0;
    virtual bool processMessages() = 0;
    virtual void getRequiredExtensions(DynamicArray<const char*>& extensions) = 0;
    virtual void ff_sleep(unsigned long ms) = 0;
    virtual void *allocate(const unsigned long size, bool align) = 0;
    virtual void freeMemory(void *memory, bool align) = 0;
    virtual void clear(void *memory, const unsigned long size) = 0;
};