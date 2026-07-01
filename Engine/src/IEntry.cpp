#include "modules/engine/Core/FF_Memory.h"
#include "modules/engine/Core/Game.h"
#include "modules/engine/Core/IGame.h"
#include "modules/engine/Core/Logger.h"

extern void createGame(IGame* game, FF_Memory& ff_memory);

int main() {
    FF_Memory ff_memory{};

    IGame instance;
    createGame(&instance, ff_memory);

    if (!instance.render || !instance.update || !instance.initialize || !instance.resize) {
        Logger::logFatal("The games function pointer must all be assigned!");
    }

    Game game{&instance, &ff_memory};

    return 0;
}
