//
// Created by cmorg on 7/1/2026.
//

#include "Engine.h"

#include "Logger.h"

void Engine::startup()
{
    if (bIsInitialized) {
        Logger::logError("Startup was already called!");
        return;
    }

    //Initialize logging
    Logger::initializeFile();
    //Initialize input
    inputSystem->initialize(*ff_memory);
    //Initialize the platform
    platform = new Platform{};

    if (!gameInstance->initialize(gameInstance)) {
        Logger::logFatal("The game failed to initialize!");
    }

    gameInstance->resize(gameInstance, width, height);

    bIsRunning = true;
    bIsPaused = false;

    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {quit(context);}, "Engine.quit", KEY_ESCAPE);

    if (!platform->initialize(gameInstance->config.appName, gameInstance->config.startingX, gameInstance->config.startingY, gameInstance->config.startingWidth, gameInstance->config.startingHeight)) {
        Logger::logFatal("The platform failed to initialize!");
        return;
    }

    bIsInitialized = true;

    run();
}

void Engine::run() {
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

            inputSystem->update(0, *ff_memory);
            platform->processInputs(*inputSystem);
        }
    }

    bIsRunning = false;
}

void Engine::quit(const EngineInputContext context) {
    Logger::logInfo("User Quit. Shutting Down.\n");
    bIsRunning = false;
}

Engine::Engine(GameInstance& instance, FF_Memory& mainMemory)
    : ff_memory(&mainMemory), inputSystem(instance.inputSystem), gameInstance(&instance)
{
    startup();
}

Engine::~Engine() {
    bIsRunning = false;
    delete platform;
}
