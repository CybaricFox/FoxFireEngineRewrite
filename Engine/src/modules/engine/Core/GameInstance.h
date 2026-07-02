#pragma once

#include <string>
#include "../../../defines.h"
#include "foxfire_export.h"
#include "IInputSystem.h"

struct GameConfig {
    String appName;
    short startingX;
    short startingY;
    short startingWidth;
    short startingHeight;
};

struct FOXFIRE_API GameInstance {
    GameConfig config;
    IInputSystem* inputSystem;

    bool (*initialize)(GameInstance* instance);
    bool (*update)(GameInstance* instance, float deltaTime);
    bool (*render)(GameInstance* instance, float deltaTime);
    bool (*resize)(GameInstance* instance, unsigned int width, unsigned int height);
    void* state;

    ~GameInstance() {
        if (state != nullptr) {
            free(state);
        }
    }
};

