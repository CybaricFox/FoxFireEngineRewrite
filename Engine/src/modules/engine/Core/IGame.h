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
};

struct FOXFIRE_API IGame {
    GameConfig config;

    bool (*initialize)(IGame* instance);
    bool (*update)(IGame* instance, float deltaTime);
    bool (*render)(IGame* instance, float deltaTime);
    bool (*resize)(IGame* instance, unsigned int width, unsigned int height);
    void* state;

    ~IGame() {
        if (state != nullptr) {
            free(state);
        }
    }
};

