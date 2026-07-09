#pragma once
#include "src/modules/engine/Core/Engine.h"

struct GameState {
    float deltaTime;
};

class Game final : public Engine{
public:
    Game();
    ~Game() override = default;

protected:
    void startup() override;
    bool update(float deltaTime) override;
};


