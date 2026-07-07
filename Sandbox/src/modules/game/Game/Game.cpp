//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/Library/Logger.h"
#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"

Game::Game(GameInstance *instance, const unsigned long size)
    : Engine(instance, size)
{
    inputSystem = new FoxFire_InputSystem();
}

Game::~Game() {

}

void Game::startup() {
    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {quit();}, "Engine.quit", KEY_ESCAPE);

    Engine::startup();
}

bool Game::update(const float deltaTime) {
    static unsigned long allocationCount = 0;
    const unsigned long previousAllocationCount = allocationCount;
    allocationCount = FF_Memory::getAllocationCount();
    if (inputSystem->isKeyUp(KEY_M) && inputSystem->wasKeyDown(KEY_M)) {
        Logger::logDebug("Allocations: " + std::to_string(allocationCount) + ". " + std::to_string(allocationCount - previousAllocationCount) + " this frame.");
    }

    return Engine::update(deltaTime);
}
