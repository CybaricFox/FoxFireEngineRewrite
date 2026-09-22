#pragma once
#include "src/modules/engine/Core/Engine.h"
#include "src/modules/system/FoxFire_Input/FoxFire_Events.h"

struct GameState : BaseGameState {
    float deltaTime = 0;
    unsigned int worldCamera = INVALID_ID_U32;
};

class Game final : public Engine{
public:
    explicit Game(const GameInstance& instance);
    ~Game() override;

    Event<void> swapTextureEvent{};

protected:
    void startup() override;
    bool update(float deltaTime) override;
    void initialize() override;
};


