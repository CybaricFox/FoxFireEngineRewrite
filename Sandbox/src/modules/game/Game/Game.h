#pragma once
#include "src/modules/engine/Core/IGame.h"

struct GameState {
    float deltaTime;
};

class Game {
public:
    static bool initialize(IGame* instance);
    static bool update(IGame* instance, float deltaTime);
    static bool render(IGame* instance, float deltaTime);
    static bool resize(IGame* instance, unsigned int width, unsigned int height);
};


