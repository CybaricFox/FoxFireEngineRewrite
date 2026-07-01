//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "Logger.h"

void Game::startup()
{
    //Logger::logFatal("Hello World!");
    Logger::logError("Hello World!");
    Logger::logWarn("Hello World!");
    Logger::logInfo("Hello World!");
    Logger::logDebug("Hello World!");

    while (true) {
        platform->processMessages();
    }
}

Game::Game()
{
    startup();
}

Game::~Game() {
    delete platform;
}
