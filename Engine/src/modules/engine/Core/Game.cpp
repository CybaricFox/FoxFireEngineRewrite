//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "Logger.h"

void Game::startup()
{
    if (bIsInitialized) {
        Logger::logError("Startup was already called!");
        return;
    }

    Logger::initializeFile();
    platform = new Platform{gameInstance->config.appName, gameInstance->config.startingX, gameInstance->config.startingY, gameInstance->config.startingWidth, gameInstance->config.startingHeight};

    if (!gameInstance->initialize(gameInstance)) {
        Logger::logFatal("The game failed to initialize!");
    }

    gameInstance->resize(gameInstance, width, height);

    Logger::logError("Hello World!");
    Logger::logWarn("Hello World!");
    Logger::logInfo("Hello World!");
    Logger::logDebug("Hello World!");
    //Logger::logFatal("Hello World!");

    bIsRunning = true;
    bIsPaused = false;

    bIsInitialized = true;
    run();
}

void Game::run() {
    Logger::logInfo(ff_memory->getMemoryUsage());

    while (bIsRunning) {
        if (!platform->processMessages()) {
            bIsRunning = false;
        }

        if (!bIsPaused) {
            if (!gameInstance->update(gameInstance, 0)) {
                Logger::logFatal("Game update tick failed!");
            }

            if (!gameInstance->render(gameInstance, 0)) {
                Logger::logFatal("Game render tick failed!");
            }
        }
    }

    bIsRunning = false;
}

Game::Game(IGame *instance, FF_Memory *mainMemory)
    : ff_memory(mainMemory), gameInstance(instance)
{
    startup();
}

Game::~Game() {
    bIsRunning = false;
    delete platform;
}
