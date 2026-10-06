/**
 *  @file Platform.h
 *  @layer Engine
 *  @module Core
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "IPlatformState.h"
#include "../Input/IInputSystem.h"
#include "src/defines.h"

/**
 * @brief Handles platform-specific operations.
 */
class Platform {
private:
    IInputSystem* inputSystemRef = nullptr;

    /** @brief stored platform state */
    static IPlatformState* platformState;

public:
    Platform() = default;
    ~Platform();

    /**
     * @brief
     * @param applicationName Name of the application.
     * @param x Starting x pos of the screen.
     * @param y Starting y pos of the screen.
     * @param width Starting width of the screen.
     * @param height Starting height of the screen.
     * @param inputSystem reference to the input system
     * @return True on success, False on failure.
     */
    bool initialize(const String &applicationName, int x, int y, int width, int height, IInputSystem* inputSystem);
    void setPlatform();
    void shutdown();

    /**
     * @brief Prints a message to console.
     * @param message The message to print
     * @param color Color of the message.
     */
    static void printConsoleMessage(const char *message, unsigned char color);

    /**
     * @brief Prints a message to console via the error stream.
     * @param message The message to print.
     * @param color The color of the message.
     */
    static void printConsoleError(const char *message, unsigned char color);

    /**
     * @brief Gets system time since EPOCH
     * @return Total time
     */
    static double getAbsoluteTime();

    /**
     * @brief Gets platform specific extensions for the render system.
     * @param extensions Dynamic array to hold the extensions.
     */
    void getRequiredExtensions(DynamicArray<const char *> &extensions);

    /**
     * @brief Returns the Platform-specific platform state as a void pointer to be used elsewhere.
     * @return The Platform-specific platform state.
     */
    [[nodiscard]] IPlatformState& getPlatformState() const {return *platformState;}

    /**
     * @brief Obtains the number of logical processor cores.
     * @return
     */
    int getProcessorCount() {return platformState->getProcessorCount();}

    /**
     * @brief Processes platform-dependent user input (Keyboard, Controller, Mouse, ETC)
     * @return False if the application should be closed (LINUX ONLY)
     */
    bool processMessages();

    /**
     * @brief Interfaces with the input system to process inputs in the order they were activated.
     */
    void processInputs() const;

    /**
     * @brief Pauses the application
     * @param ms Time to stay paused
     */
    static void ff_sleep(unsigned long ms);

    /**
     * @brief Creates render surface from os.
     * @return False on failure.
     */
    [[nodiscard]] bool createSurface() const;

    /**
     * @brief Asks the OS to allocate memory. Avoid calling this. Use FF_Memory instead.
     * @param size size of memory to allocate
     * @param align Whether the memory should be aligned.
     * @return Pointer to the new memory block.
     */
    static void *platform_allocate(unsigned long size, bool align);

    /**
     * @brief Asks the OS to free memory. Only use this if memory was not allocated by FF_Memory or it will break FF_Memory.
     * @param memory pointer to the memory location
     * @param align Whether the memory is aligned.
     */
    static void platform_free(void* memory, bool align);

    /**
     * @brief Zeros out memory.
     * @param memory Pointer to the memory location.
     * @param size size to clear out.
     */
    static void platform_clear(void* memory, unsigned long size);

    static bool createMutex(Mutex& mutex);
    static void destroyMutex(Mutex& mutex);
    static bool lockMutex(Mutex& mutex);
    static bool unlockMutex(Mutex& mutex);
    static ULong getCurrentThread() {return platformState->getCurrentThreadId();}
    static bool createThread(const ThreadFunction threadFunction, void* params, const bool autoDetach, Thread& outThread) {return platformState->createThread(threadFunction, params, autoDetach, outThread);}
    static void destroyThread(Thread& thread) {platformState->destroyThread(thread);}
    static void pauseThread(Thread& thread, const ULong ms) {platformState->pauseThread(thread, ms);}

};