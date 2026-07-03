//
// Created by cmorg on 7/1/2026.
//

#include "Engine.h"

#include "../Library/Logger.h"

void Engine::startup()
{
    if (bIsInitialized) {
        Logger::logError("Startup was already called!");
        return;
    }

    Logger::logInfo("Beginning startup sequence");

    resize(width, height);

    bIsRunning = true;
    bIsPaused = false;

    if (!platform->initialize(gameInstance->config.appName, gameInstance->config.startingX, gameInstance->config.startingY, gameInstance->config.startingWidth, gameInstance->config.startingHeight)) {
        Logger::logFatal("The platform failed to initialize!");
        return;
    }

    bIsInitialized = true;

    //Start renderer
    if (!renderer.initialize(gameInstance->config.appName, platform, backend, gameInstance, &ff_memory)) {
        Logger::logFatal("Failed to initialize renderer!");
        return;
    }

    run();
}

void Engine::run() {
    Logger::logInfo("Beginning run loop");

    clock.start(*platform);
    clock.update(*platform);
    lastTime = clock.getElapsedTime();
    double runTime = 0;
    unsigned char frameCount = 0;
    constexpr double targetTime = 1.0f / 60;

    int fps = 0;
    float deltaCount = 0;

    Logger::logInfo(ff_memory.getMemoryUsage());

    while (bIsRunning) {
        if (!platform->processMessages()) {
            bIsRunning = false;
        }

        if (!bIsPaused) {
            //Update clock
            clock.update(*platform);
            const double currentTime = clock.getElapsedTime();
            const double deltaTime = (currentTime - lastTime);
            const double frameStartTime = platform->getAbsoluteTime();

            //Logger::logInfo(std::to_string(deltaTime));

            if (!update(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game update tick failed!");
            }

            if (!render(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game render tick failed!");
            }

            RenderPacket packet{};
            packet.deltaTime = static_cast<float>(deltaTime);

            renderer.drawFrame(&packet, backend);

            //How long did the frame take
            const double endTime = platform->getAbsoluteTime();
            const double elapsedTime = endTime - frameStartTime;
            runTime += elapsedTime;
            const double remainingTime = targetTime - elapsedTime;

            //Time left is given back to the OS
            if (remainingTime > 0) {
                const unsigned long remainingMS = remainingTime * 1000;
                constexpr bool limitFrames = false;
                if (remainingMS > 0 && limitFrames) {
                    platform->ff_sleep(remainingMS - 1);
                }

                frameCount++;
            }

            //Handle input at the end
            platform->processInputs(*inputSystem);
            inputSystem->update(deltaTime, ff_memory);

            //Update last time
            lastTime = currentTime;

            if (deltaCount >= 1) {
                Logger::logInfo("FPS: " + std::to_string(fps));
                fps = 0;
                deltaCount = 0;
            } else {
                fps++;
                deltaCount += static_cast<float>(deltaTime);
            }
        }
    }

    bIsRunning = false;
}

void Engine::resize(unsigned int width, unsigned int height) {
}

bool Engine::update(float deltaTime) {
    return true;
}

bool Engine::render(float deltaTime) {
    return true;
}

Engine::Engine(GameInstance *instance, const unsigned long stateSize) {
    instance->state = ff_memory.ff_allocate(stateSize, GAME);
}

void Engine::quit(const EngineInputContext context) {
    Logger::logInfo("User Quit. Shutting Down.\n");
    bIsRunning = false;
}

void Engine::initialize(GameInstance &instance) {
    if (bIsInitialized) {
        Logger::logError("Initialize was already called!");
        return;
    }

    Logger::initializeFile();

    Logger::logInfo("Initializing Game");

    gameInstance = &instance;
    inputSystem->initialize(ff_memory);
    platform = new Platform{};

    startup();
}

Engine::~Engine() {
    bIsRunning = false;

    //Destroy resources in opposite order of creation
    delete backend;
    delete platform;
    delete inputSystem;
}
