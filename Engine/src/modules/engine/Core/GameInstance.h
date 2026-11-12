/**
 *  @file GameInstance.h
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

#include "foxfire_export.h"
#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Renderer/FontSystem.h"
#include "src/modules/engine/Renderer/IRenderView.h"

struct BaseGameState {};

/**
 * @brief Config data for this application set by the user
 */
struct GameConfig {
    /** @brief Name of the application */
    const char* appName{};
    /** @brief Starting x screen position of the window */
    short startingX = 0;
    /** @brief Starting y height position of the window */
    short startingY = 0;
    /** @brief Starting width of the window */
    short startingWidth = 0;
    /** @brief Starting height of the window */
    short startingHeight = 0;

    int gameVersionMajor = 0;
    int gameVersionMinor = 0;
    int gameVersionPatch = 0;

    FontSystemConfig fontConfig{};
    DynamicArray<RenderViewConfig> renderViews{};

    GameConfig& operator= (const GameConfig& other) {
        if (this == &other) return *this;

        appName = other.appName;
        startingX = other.startingX;
        startingY = other.startingY;
        startingWidth = other.startingWidth;
        startingHeight = other.startingHeight;
        gameVersionMajor = other.gameVersionMajor;
        gameVersionMinor = other.gameVersionMinor;
        gameVersionPatch = other.gameVersionPatch;
        fontConfig = other.fontConfig;

        return *this;
    }

    void shutdown() {
        renderViews.shutdown();
    }
};

/**
 * @brief Game-specific config data set by the user
 */
struct FOXFIRE_API GameInstance {
    /** @brief Contains application startup data */
    GameConfig config{};

    /** @brief Game State struct defined by the user */
    BaseGameState* state = nullptr;
    unsigned long memoryRequirement = 0;

    void shutdown() {
        if (!state) return;
        FF_Memory::ff_free<BaseGameState>(state, GAME, memoryRequirement); //Game state does not use destruction, so ff_free is valid.
        state = nullptr;
        config.shutdown();
    }
};

