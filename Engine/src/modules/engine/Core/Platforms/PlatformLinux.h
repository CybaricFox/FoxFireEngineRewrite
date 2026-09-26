//
// Created by cmorg on 9/26/2026.
//

#pragma once
#include "src/modules/engine/Core/PlatformState.h"

/**
 *  @file PlatformLinux.h
 *  @layer Engine
 *  @module Core
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/26/2026
 *
 *  @copyright (c) 2026
 */

#if FOXFIRE_PLATFORM_LINUX == 1

#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <cstring>

#if _POSIX_C_SOURCE >= 199309L
    #include <ctime>
#else
    #include <unistd.h>
#endif

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include "src/modules/engine/Renderer/Vulkan/VulkanBackend.h"

class PlatformLinux final : public PlatformState {
private:
    Display* display = nullptr;
    xcb_connection_t* connection = nullptr;
    xcb_window_t window{};
    xcb_screen_t* screen = nullptr;
    xcb_atom_t wm_protocols{};
    xcb_atom_t wm_delete_win{};
    VkSurfaceKHR surface{};

public:
    explicit PlatformLinux()
        : PlatformState(sizeof(PlatformLinux)) {
    }

private:
    Keys translateKeycode(unsigned int keyCode);

public:
    void printConsoleMessage(const String &message, unsigned char color) override;
    void printConsoleError(const String &message, unsigned char color) override;
    bool createSurface() override;
    double getAbsoluteTime() override;
    bool processMessages() override;
    void getRequiredExtensions(DynamicArray<const char *> &extensions) override;
    void ff_sleep(unsigned long ms) override;

    bool initialize(const String &applicationName, const int x, const int y, const int width, const int height) override;

    void shutdown() override;
    void * allocate(const unsigned long size, bool align) override;
    void freeMemory(void *memory, bool align) override;
    void clear(void *memory, const unsigned long size) override;
};

#endif