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

    resize(gameInstance->config.startingWidth, gameInstance->config.startingHeight);

    //Setup builtin engine events
    inputSystem->subscribeToEngineEvent(QUIT, [this](const EngineInputContext context) {quit();}, "Static.quit");
    inputSystem->subscribeToEngineEvent(RESIZED, [this](const EngineInputContext context) {resize(context.mouseX, context.mouseY);}, "Static.resize");

    bIsInitialized = true;
    bIsRunning = true;
    bIsPaused = false;

    run();
}

void Engine::run() {
    Logger::logInfo("Beginning run loop");

    clock.start(platform);
    clock.update(platform);
    lastTime = clock.getElapsedTime();
    double runTime = 0;
    unsigned char frameCount = 0;
    constexpr double targetTime = 1.0f / 60;

    int fps = 0;
    float deltaCount = 0;

    //Debug gets memory usage before starting the run loop
    //note that memory usage only shows tracked memory, not full memory.
    Logger::logInfo(FF_Memory::getMemoryUsage());

    while (bIsRunning) {
        //Input detection.
        if (!platform.processMessages()) {
            bIsRunning = false;
        }

        if (!bIsPaused) {
            //Update clock
            clock.update(platform);
            const double currentTime = clock.getElapsedTime();
            const double deltaTime = currentTime - lastTime;
            const double frameStartTime = Platform::getAbsoluteTime();

            if (!engine->update(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game update tick failed!");
            }

            if (!render(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game render tick failed!");
            }

            RenderPacket packet{};
            packet.deltaTime = static_cast<float>(deltaTime);

            masterRenderSystem.drawFrame(packet, *backend);

            //How long did the frame take
            const double endTime = Platform::getAbsoluteTime();
            const double elapsedTime = endTime - frameStartTime;
            runTime += elapsedTime;
            const double remainingTime = targetTime - elapsedTime;
            //Time left is given back to the OS
            if (remainingTime > 0) {
                const unsigned long remainingMS = remainingTime * 1000;
                constexpr bool limitFrames = false;
                if (remainingMS > 0 && limitFrames) {
                    platform.ff_sleep(remainingMS - 1);
                }

                frameCount++;
            }
            //Handle input at the end
            //Must be update -> processInputs or key state wont be tracked correctly
            inputSystem->update(deltaTime);
            platform.processInputs(*inputSystem);

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

void Engine::resize(const unsigned short newWidth, const unsigned short newHeight) {
    if (width != newWidth || height != newHeight) {
        width = newWidth;
        height = newHeight;

        if (width == 0 || height == 0) {
            Logger::logInfo("window minimize, Suspending application.");
            bIsPaused = true;
        } else {
            if (bIsPaused) {
                Logger::logInfo("Window restored, resuming application.");
                bIsPaused = false;
            }

            masterRenderSystem.onResize(width, height, backend);
        }
    }
}

bool Engine::update(float deltaTime) {
    return true;
}

bool Engine::render(float deltaTime) {
    return true;
}

Engine::Engine()
{
    initializeMemory();
    Logger::initializeFile(logHandler);
}

void Engine::initializeMemory() {
    FF_Memory::initialize(memoryDataStore);

    unsigned long totalMemorySize = 0;

    linearAllocator.initialize(totalMemorySize, nullptr);
}

void Engine::initializeRenderSystem() {
    if (!masterRenderSystem.initialize(gameInstance->config.appName, platform, backend, *gameInstance, width, height)) {
        Logger::logFatal("Failed to initialize renderer!");
    }
}

void Engine::quit() {
    Logger::logInfo("User Quit. Shutting Down.\n");
    bIsRunning = false;
}

void Engine::initialize(GameInstance &instance) {
    if (bIsInitialized) {
        Logger::logError("Initialize was already called!");
        return;
    }

    Logger::logInfo("Initializing Game");

    //Initialize event and input systems
    gameInstance = &instance;
    EngineEvents::initialize(subscribers);
    inputSystem->initialize();

    //Initialize platform class
    if (!platform.initialize(gameInstance->config.appName, gameInstance->config.startingX, gameInstance->config.startingY, gameInstance->config.startingWidth, gameInstance->config.startingHeight)) {
        Logger::logFatal("The platform failed to initialize!");
        return;
    }

    //Start renderer
    initializeRenderSystem();

    startup();
}

void Engine::getFramebufferSize(unsigned int& bufferWidth, unsigned int& bufferHeight) const {
    bufferWidth = width;
    bufferHeight = height;
}

Engine::~Engine() {
    Logger::logInfo(FF_Memory::getMemoryUsage());

    bIsRunning = false;
    engine = nullptr;

    //Static destruction
    EngineEvents::shutdown();

    //Destroy resources in opposite order of creation
    delete backend;
    if (inputSystem) {
        delete inputSystem;
        inputSystem = nullptr;
    }

    platform.shutdown();

    linearAllocator.shutdown();
    //THIS MUST ALWAYS SHUTDOWN LAST!
    FF_Memory::shutdown();
    //cleanup logger after memory shutdown so memory errors output to log file.
    Logger::cleanup();
}

void Engine::setEngineRef(Engine& derivedEngine) {
    if (engine != nullptr) return;

    engine = &derivedEngine;
}
