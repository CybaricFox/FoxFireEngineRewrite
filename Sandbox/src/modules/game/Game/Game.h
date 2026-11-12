#pragma once
#include "src/modules/engine/Core/Engine.h"
#include "../../../../../Engine/src/modules/engine/Input/EventSystem.h"
#include "src/modules/engine/Resources/Skybox.h"

struct GameState : BaseGameState {
    float deltaTime = 0;
    unsigned int worldCamera = INVALID_ID_U32;
    unsigned short width = 0;
    unsigned short height = 0;
    Skybox skybox{};
    unsigned int worldEntities[10]{};
    unsigned int uiEntities[10]{};
    unsigned int maxwell = INVALID_ID_U32;
    bool bModelsLoaded = false;
    UIText bitmapText{};
    UIText systemText{};
    unsigned int hoveredId = 0;
};

class Game final : public Engine {
private:
    void loadModels();
    bool configureRenderViews();
public:
    explicit Game(const GameInstance& instance);
    ~Game() override;

    Event<void> swapTextureEvent{};
    Event<void> loadModelsEvent{};

protected:
    void startup() override;
    bool update(float deltaTime) override;
    void initialize() override;

    bool boot() override;

    bool render(float deltaTime, RenderPacket &renderPacket) override;
    void resize(unsigned short newWidth, unsigned short newHeight) override;
};


