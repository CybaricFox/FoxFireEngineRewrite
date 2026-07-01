//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/Core/Logger.h"


bool Game::initialize(IGame *instance) {
    Logger::logDebug("Initializing...");
    return true;
}

bool Game::update(IGame *instance, float deltaTime) {
    //Logger::logDebug("Updating...");
    return true;
}

bool Game::render(IGame *instance, float deltaTime) {
    //Logger::logDebug("Rendering...");
    return true;
}

bool Game::resize(IGame *instance, unsigned int width, unsigned int height) {
    Logger::logDebug("Resizing...");
    return true;
}
