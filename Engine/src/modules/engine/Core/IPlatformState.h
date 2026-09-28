//
// Created by cmorg on 9/26/2026.
//

#pragma once
#include "src/modules/engine/Input/IInputSystem.h"
#include "src/modules/engine/Threads/Mutex.h"
#include "src/modules/engine/Threads/Thread.h"

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

class IPlatformState {
private:
    ULong size = 0;

protected:
    DynamicArray<KeyState> keyInputs{};
    DynamicArray<MouseState> mouseInputs{};

public:
    explicit IPlatformState(const ULong newSize) : size(newSize) {};
    virtual ~IPlatformState() = default;

    [[nodiscard]] ULong getSize() const { return size; }
    virtual int getProcessorCount() = 0;

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
    virtual void *allocate(unsigned long size, bool align) = 0;
    virtual void freeMemory(void *memory, bool align) = 0;
    virtual void clear(void *memory, unsigned long size) = 0;
    virtual bool createThread(ThreadFunction threadFunction, void* params, bool autoDetach, Thread& outThread) = 0;
    virtual void destroyThread(Thread& thread) = 0;
    virtual void cancelThread(Thread& thread) = 0;
    virtual void detachThread(Thread& thread) = 0;
    virtual bool isThreadActive(Thread& thread) = 0;
    virtual void pauseThread(Thread& thread, ULong ms) = 0;
    virtual ULong getCurrentThreadId() = 0;
    virtual bool createMutex(Mutex& outMutex) = 0;
    virtual void destroyMutex(Mutex& mutex) = 0;
    virtual bool lockMutex(Mutex& mutex) = 0;
    virtual bool unlockMutex(Mutex& mutex) = 0;
};