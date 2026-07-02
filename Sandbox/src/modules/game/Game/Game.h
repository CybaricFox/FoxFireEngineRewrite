#pragma once
#include "src/modules/engine/Core/Engine.h"
#include "src/modules/engine/Core/GameInstance.h"

struct GameState {
    float deltaTime;
};

class Game final : public Engine{
public:
    Game(GameInstance* instance, unsigned long size);
    ~Game() override;

    void initialize(GameInstance* instance);

protected:
    void startup() override;
};


