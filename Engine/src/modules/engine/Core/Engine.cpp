//
// Created by cmorg on 7/1/2026.
//

#include "Engine.h"

#include "../Library/Logger.h"
#include "src/modules/engine/Memory/DynamicAllocator.h"

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
                bIsRunning = false;
            }

            if (!render(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game render tick failed!");
                bIsRunning = false;
            }

            RenderPacket packet{};
            packet.deltaTime = static_cast<float>(deltaTime);

            //temp code
            GeometryRenderData testData{};
            testData.geometry = testGeometry;
            testData.model = matrixIdentity();
            packet.geometryCount = 1;
            packet.geometries = &testData;

            GeometryRenderData testUIData{};
            testUIData.geometry = testUIGeometry;
            testUIData.model = createTranslationMatrix({0, 0, 0});
            packet.uiGeometryCount = 1;
            packet.uiGeometries = &testUIData;
            //end temp code

            if (!masterRenderSystem.drawFrame(packet)) {
                Logger::logFatal("Failed to draw frame!");
                bIsRunning = false;
            }

            //How long did the frame take
            const double endTime = Platform::getAbsoluteTime();
            const double elapsedTime = endTime - frameStartTime;
            runTime += elapsedTime;
            const double remainingTime = targetTime - elapsedTime;
            //Time left is given back to the OS
            if (remainingTime > 0) {
                const unsigned long remainingMS = static_cast<unsigned long>(remainingTime) * 1000;
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

            masterRenderSystem.onResize(width, height);
        }
    }
}

bool Engine::update(float deltaTime) {
    return true;
}

bool Engine::render(float deltaTime) {
    return true;
}

void Engine::setView(const Mat4 &newView) {
    masterRenderSystem.setView(newView);
}

Engine::Engine(GameInstance& instance)
{
    if (!initializeMemory()) throw;
    Logger::initializeFile(logHandler);
    instance.state = FF_Memory::ff_allocate(instance.memoryRequirement, GAME);
    gameInstance = &instance;
}

bool Engine::initializeMemory() {
    MemoryConfig config{};
    config.totalAllocationSize = GIBIBYTES(1);
    if (!FF_Memory::initialize(config)) {
        Logger::logError("Failed to initialize memory!");
        return false;
    }

    return true;
    //linearAllocator.initialize(totalMemorySize, nullptr);
}

void Engine::quit() {
    Logger::logInfo("User Quit. Shutting Down.\n");
    bIsRunning = false;
}

void Engine::initialize() {
    if (bIsInitialized) {
        Logger::logError("Initialize was already called!");
        return;
    }

    Logger::logInfo("Initializing Game");

    FF_Memory::ff_allocate(gameInstance->memoryRequirement, GAME);

    //Initialize event and input systems
    EngineEvents::initialize(subscribers);
    inputSystem->initialize();

    //Initialize platform class
    if (!platform.initialize(gameInstance->config.appName, gameInstance->config.startingX, gameInstance->config.startingY, gameInstance->config.startingWidth, gameInstance->config.startingHeight)) {
        Logger::logFatal("The platform failed to initialize!");
        return;
    }

    //Initialize the resource system
    if (!resourceSystem.initialize("Assets", 32)) {
        Logger::logFatal("Failed to initialize the resource system!");
        return;
    }

    //Start renderer
    if (!masterRenderSystem.initialize(gameInstance->config.appName, platform, *gameInstance, width, height, resourceSystem)) {
        Logger::logFatal("Failed to initialize the render system!");
        return;
    }

    //Start texture system
    if (!masterRenderSystem.initializeTextureSystem(65536, textureSystem, &resourceSystem)) {
        Logger::logFatal("Failed to initialize the texture system!");
        return;
    }

    //Start the shader system
    if (!masterRenderSystem.initializeShaderSystem(ShaderSystemConfig{1024, 128, 31, 31}, resourceSystem)) {
        Logger::logFatal("Failed to initialize the shader system!");
    }

    //Start material system
    if (!masterRenderSystem.initializeMaterialSystem(MaterialSystemConfig{4096}, materialSystem, &resourceSystem)) {
        Logger::logFatal("Failed to initialize the texture system!");
        return;
    }

    //Start geometry system
    if (!masterRenderSystem.initializeGeometrySystem(4096, geometrySystem, &resourceSystem)) {
        Logger::logFatal("Failed to initialize the geometry system!");
        return;
    }

    //Temp code
    const GeometryConfig config = masterRenderSystem.generatePlaneConfig(10, 10, 5, 5, 2, 2, "test geometry", "MaterialTemplate");
    testGeometry = &masterRenderSystem.acquireGeometry(config, true);
    FF_Memory::ff_free(config.vertices, sizeof(Vertex3d) * config.vertexCount, ARRAY);
    FF_Memory::ff_free(config.indices, sizeof(unsigned int) * config.indexCount, ARRAY);

    //Ui geo
    GeometryConfig configUI{};
    configUI.vertexSize = sizeof(Vertex2d);
    configUI.vertexCount = 4;
    configUI.indexSize = sizeof(unsigned int);
    configUI.indexCount = 6;
    configUI.materialName = "GenericUI";
    configUI.name = "test ui geometry";

    constexpr float f = 512;
    Vertex2d uiVerts[4];
    uiVerts[0].position.x = 0;
    uiVerts[0].position.y = 0;
    uiVerts[0].textureCoordinate.x = 0;
    uiVerts[0].textureCoordinate.y = 0;
    uiVerts[1].position.x = f;
    uiVerts[1].position.y = f;
    uiVerts[1].textureCoordinate.x = 1;
    uiVerts[1].textureCoordinate.y = 1;
    uiVerts[2].position.x = 0;
    uiVerts[2].position.y = f;
    uiVerts[2].textureCoordinate.x = 0;
    uiVerts[2].textureCoordinate.y = 1;
    uiVerts[3].position.x = f;
    uiVerts[3].position.y = 0;
    uiVerts[3].textureCoordinate.x = 1;
    uiVerts[3].textureCoordinate.y = 0;
    configUI.vertices = uiVerts;

    unsigned int uiIndices[6] = {2, 1, 0, 3, 0, 1};
    configUI.indices = uiIndices;

    testUIGeometry = &masterRenderSystem.acquireGeometry(configUI, true);
    //End temp code

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
    gameInstance->shutdown();

    //Destroy resources in opposite order of creation
    geometrySystem = nullptr;
    materialSystem = nullptr;
    textureSystem = nullptr;
    masterRenderSystem.shutdown();

    if (inputSystem) {
        delete inputSystem;
        inputSystem = nullptr;
    }

    resourceSystem.shutdown();

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
