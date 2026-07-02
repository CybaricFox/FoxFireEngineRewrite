#include "modules/engine/Core/FF_Memory.h"
#include "modules/engine/Core/Engine.h"
#include "modules/engine/Core/GameInstance.h"
#include "modules/engine/Core/Logger.h"

extern void createGame(GameInstance* game, FF_Memory& ff_memory);

int main() {
    FF_Memory ff_memory{};

    GameInstance instance;
    createGame(&instance, ff_memory);

    if (!instance.render || !instance.update || !instance.initialize || !instance.resize) {
        Logger::logFatal("The games function pointer must all be assigned!");
    }

    Engine engine{instance, ff_memory};

    return 0;
}
