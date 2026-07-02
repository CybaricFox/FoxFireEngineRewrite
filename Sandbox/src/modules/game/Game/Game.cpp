//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"

Game::Game(GameInstance *instance, const unsigned long size)
    : Engine(instance, size)
{
    inputSystem = new FoxFire_InputSystem();
}

Game::~Game() {
    Engine::~Engine();
}

void Game::startup() {
    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {quit(context);}, "Engine.quit", KEY_ESCAPE);

    Engine::startup();
}
