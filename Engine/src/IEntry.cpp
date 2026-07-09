#include "modules/engine/Core/Engine.h"
#include "modules/engine/Core/GameInstance.h"
#include "modules/engine/Library/Logger.h"

extern void createGame(GameInstance& game, Engine*& engine);

int main() {
    GameInstance instance{};

    Engine* engine = nullptr;

    createGame(instance, engine);
    engine->setEngineRef(*engine);

    if (engine != nullptr) {
        if (instance.state == nullptr) {
            Logger::logError("Engine instance is null!");
        }
        engine->initialize(instance);
    }

    delete engine;

    return 0;
}
