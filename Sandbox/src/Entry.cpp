#include <cstdlib>

#include "src/modules/engine/Core/IGame.h"
#include "modules/game/Game/Game.h"
#include "src/modules/engine/Core/FF_Memory.h"

//Function that creates the game on the engine side
void createGame(IGame* instance, FF_Memory& ff_memory) {
    instance->config.startingX = 100;
    instance->config.startingY = 100;
    instance->config.startingWidth = 1280;
    instance->config.startingHeight = 720;
    instance->config.appName = "FoxFire Engine Sandbox";

    instance->initialize = Game::initialize;
    instance->update = Game::update;
    instance->render = Game::render;
    instance->resize = Game::resize;

    instance->state = ff_memory.ff_allocate(sizeof(GameState), GAME);
}
