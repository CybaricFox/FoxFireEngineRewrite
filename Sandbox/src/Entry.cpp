#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Engine.h"

#include "modules/game/Game/Game.h"

//Function that creates the game on the engine side
void createGame(GameInstance& instance, Engine*& engine) {
    instance.config.startingX = 100;
    instance.config.startingY = 100;
    instance.config.startingWidth = 1280;
    instance.config.startingHeight = 720;
    instance.config.appName = "FoxFire Engine Sandbox";

    instance.config.gameVersionMajor = 0;
    instance.config.gameVersionMinor = 0;
    instance.config.gameVersionPatch = 0;

    instance.memoryRequirement = sizeof(GameState);

    engine = new Game(instance);
}
