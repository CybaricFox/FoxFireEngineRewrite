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

#include "../Input/EngineEvents.h"
#include "../Input/IInputSystem.h"
#include "src/defines.h"

/**
 * @brief Platform data is different per Platform. This stuct contains a pointer to the platform-specific state.
 * The platform state can be found in the .cpp file.
 */
struct PlatformState {
    void* unknownState = nullptr;
};

/**
 * @brief Handles platform-specific operations.
 */
class Platform {
public:
    Platform() = default;
    ~Platform();
    void shutdown();

    /**
     * @brief Prints a message to console.
     * @param message The message to print
     * @param color Color of the message.
     */
    static void printConsoleMessage(const String& message, unsigned char color);

    /**
     * @brief Prints a message to console via the error stream.
     * @param message The message to print.
     * @param color The color of the message.
     */
    static void printConsoleError(const String& message, unsigned char color);

    /**
     * @brief Processes platform-dependent user input (Keyboard, Controller, Mouse, ETC)
     * @return False if the application should be closed (LINUX ONLY)
     */
    bool processMessages();

    /**
     * @brief Gets system time since EPOCH
     * @return Total time
     */
    static double getAbsoluteTime();

    /**
     * @brief Interfaces with the input system to process inputs in the order they were activated.
     * @param inputSystem Reference to the input system.
     */
    void processInputs(IInputSystem& inputSystem);

    /**
     * @brief
     * @param applicationName Name of the application.
     * @param x Starting x pos of the screen.
     * @param y Starting y pos of the screen.
     * @param width Starting width of the screen.
     * @param height Starting height of the screen.
     * @return True on success, False on failure.
     */
    bool initialize(const String &applicationName, int x, int y, int width, int height);

    /**
     * @brief Pauses the application
     * @param ms Time to stay paused
     */
    void ff_sleep(unsigned long ms);

    void getRequiredExtensions(DynamicArray<const char *> &extensions);

    /**
     * @brief Returns the Platform-specific platform state as a void pointer to be used elsewhere.
     * @return The Platform-specific platform state.
     */
    PlatformState& getPlatformState() {return platformState;}

    bool createSurface();

    static void *platform_allocate(unsigned long size, bool align);

    static void platform_free(void* memory, bool align);
    static void platform_clear(void* memory, unsigned long size);

private:
    /** @brief stored platform state */
    PlatformState platformState{};

};


