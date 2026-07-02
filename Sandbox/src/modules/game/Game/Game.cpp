//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/Core/Logger.h"

FoxFire_InputSystem Game::inputSystem{};

bool Game::initialize(GameInstance *instance) {
    Logger::logDebug("Initializing Application");

    return true;
}

bool Game::update(GameInstance *instance, float deltaTime) {
    //Logger::logDebug("Updating Application");
    return true;
}

bool Game::render(GameInstance *instance, float deltaTime) {
    //Logger::logDebug("Rendering Application");
    return true;
}

bool Game::resize(GameInstance *instance, unsigned int width, unsigned int height) {
    Logger::logDebug("Resizing Application");
    return true;
}
