#pragma once
#include "../../system/FoxFire_Input/FoxFire_Events.h"
#include "src/modules/engine/Core/Engine.h"

struct GameState {
    float deltaTime;
    Mat4 view;
    Vector3f cameraPos;
    Vector3f cameraEuler;
    bool bIsCameraDirty;
};

class Game final : public Engine{
private:
    void recalculateView(GameState* state);
    void increaseCameraYaw(GameState* state, float amount);
    void increaseCameraPitch(GameState* state, float amount);
    void increaseCameraRoll(GameState* state, float amount);

public:
    Game();
    ~Game() override;

    EventData<void> swapTextureEvent{};

protected:
    void startup() override;
    bool update(float deltaTime) override;
    void initialize(GameInstance &instance) override;
};


