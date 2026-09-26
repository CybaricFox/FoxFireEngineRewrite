//
// Created by cmorg on 9/26/2026.
//

#pragma once
#include "src/modules/engine/Core/PlatformState.h"

#if FOXFIRE_PLATFORM_WINDOWS == 1

#include "windows.h"
#include "windowsx.h"

//REMOVE VULKAN FROM THIS EVENTUALLY!
#include "../../Renderer/Vulkan/VulkanBackend.h"
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_win32.h"

/**
 *  @file PlatformWindows.h
 *  @layer Engine
 *  @module Core
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/26/2026
 *
 *  @copyright (c) 2026
 */

class PlatformWindows final : public PlatformState{
private:
    HINSTANCE instance{};
    HWND hwnd{};
    VkSurfaceKHR surface{};
    double clockFrequency = 0;
    LARGE_INTEGER startTime{};

    void setupClock();

public:
    static PlatformWindows* self;

    PlatformWindows();

    bool initialize(const String &applicationName, const int x, const int y, const int width, const int height) override;
    void addKeyInput(Buttons button, Keys key, bool isPressed);
    void addMouseInput(int x, int y, int z);

    void printConsoleMessage(const String &message, unsigned char color) override;
    void printConsoleError(const String &message, unsigned char color) override;
    bool createSurface() override;
    double getAbsoluteTime() override;
    void shutdown() override;
    bool processMessages() override;
    void getRequiredExtensions(DynamicArray<const char*>& extensions) override;
    void ff_sleep(unsigned long ms) override;

    void *allocate(const unsigned long size, bool align) override;
    void freeMemory(void *memory, bool align) override;
    void clear(void *memory, const unsigned long size) override;
};

#endif