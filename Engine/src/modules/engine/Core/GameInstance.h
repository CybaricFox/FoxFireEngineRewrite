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

#include <string>
#include "../../../defines.h"
#include "foxfire_export.h"
#include "src/modules/engine/Memory/FF_Memory.h"

struct BaseGameState {};

/**
 * @brief Config data for this application set by the user
 */
struct GameConfig {
    /** @brief Name of the application */
    String appName;
    /** @brief Starting x screen position of the window */
    short startingX;
    /** @brief Starting y height position of the window */
    short startingY;
    /** @brief Starting width of the window */
    short startingWidth;
    /** @brief Starting height of the window */
    short startingHeight;

    int gameVersionMajor;
    int gameVersionMinor;
    int gameVersionPatch;
};

/**
 * @brief Game-specific config data set by the user
 */
struct FOXFIRE_API GameInstance {
    GameConfig config{};

    /** @brief Game State struct defined by the user */
    BaseGameState* state = nullptr;
    unsigned long memoryRequirement = 0;

    void shutdown() {
        if (!state) return;
        FF_Memory::ff_free(state, memoryRequirement, GAME); //Game state does not use destruction, so ff_free is valid.
        state = nullptr;
    }
};

