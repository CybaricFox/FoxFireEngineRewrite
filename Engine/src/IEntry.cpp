#include "modules/engine/Library/FF_Memory.h"
#include "modules/engine/Core/Engine.h"
#include "modules/engine/Core/GameInstance.h"

extern void createGame(GameInstance* game, Engine*& engine);

int main() {
    GameInstance instance;
    Engine* engine = nullptr;

    createGame(&instance, engine);

    if (engine != nullptr) {
        engine->initialize(instance);
    }

    return 0;
}
