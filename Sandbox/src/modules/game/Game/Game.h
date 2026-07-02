#pragma once
#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"

struct GameState {
    float deltaTime;
};

class Game{
public:
    static FoxFire_InputSystem inputSystem;

    static bool initialize(GameInstance* instance);
    static bool update(GameInstance* instance, float deltaTime);
    static bool render(GameInstance* instance, float deltaTime);
    static bool resize(GameInstance* instance, unsigned int width, unsigned int height);
};


