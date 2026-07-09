#pragma once

#include <string>
#include "../../../defines.h"
#include "foxfire_export.h"

struct GameConfig {
    String appName;
    short startingX;
    short startingY;
    short startingWidth;
    short startingHeight;

    int gameVersionMajor;
    int gameVersionMinor;
    int gameVersionPatch;
};

struct FOXFIRE_API GameInstance {
    GameConfig config;

    //Game State
    void* state;

    ~GameInstance() {
        if (state != nullptr) {
            free(state);
            state = nullptr;
        }
    }
};

