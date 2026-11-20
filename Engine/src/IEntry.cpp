#include "modules/engine/Core/Engine.h"
#include "modules/engine/Core/GameInstance.h"

extern void createApplication(GameInstance& game, Engine*& engine);

int main() {
    GameInstance instance{};

    Engine* engine = nullptr;

    createApplication(instance, engine);
    engine->setEngineRef(*engine);

    if (engine != nullptr) {
        engine->initialize();
    }

    delete engine;

    return 0;
}
