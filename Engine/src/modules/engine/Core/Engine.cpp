//
// Created by cmorg on 7/1/2026.
//

#include "Engine.h"

#include "../Library/Logger.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/ECS/Engine_Components/UITextContainer.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/MeshUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Library/JsonHandler.h"
#include "src/modules/engine/Library/UUID.h"
#include "src/modules/system/FoxFire_Renders/WorldRenderView.h"

void Engine::startup()
{
    if (bIsInitialized) {
        Logger::logError("Startup was already called!");
        return;
    }

    Logger::logInfo("Beginning startup sequence");

    resize(gameInstance.config.startingWidth, gameInstance.config.startingHeight);

    //Setup builtin engine events
    EngineEvents::subscribe(QUIT, [this](const EngineInputContext context) {quit();}, "Static.quit");
    EngineEvents::subscribe(RESIZED, [this](const EngineInputContext context) {resize(context.mouseX, context.mouseY);}, "Static.resize");
    //Setup hover event here when ready

    bIsInitialized = true;
    bIsRunning = true;
    bIsPaused = false;

    run();
}

void Engine::run() {
    Logger::logInfo("Beginning run loop");

    clock.start();
    clock.update();
    lastTime = clock.getElapsedTime();
    unsigned char frameCount = 0;
    constexpr double targetTime = 1.0f / 60;
    double elapsedTime = 0;

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
            clock.update();
            const double currentTime = clock.getElapsedTime();
            const double deltaTime = currentTime - lastTime;
            const double frameStartTime = Platform::getAbsoluteTime();

            //Updates jobs
            jobSystem.update();

            engineMetrics.update(elapsedTime);

            if (!engine->update(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game update tick failed!");
                bIsRunning = false;
            }

            RenderPacket packet{};
            packet.deltaTime = static_cast<float>(deltaTime);

            if (!engine->render(static_cast<float>(deltaTime), packet)) {
                Logger::logFatal("Render tick failed!");
                bIsRunning = false;
                break;
            }

            if (!masterRenderSystem.drawFrame(packet)) {
                Logger::logFatal("Failed to draw frame!");
                bIsRunning = false;
            }

            //Cleanup Packet
            for (unsigned int i = 0; i < packet.viewCount; i++) {
                packet.views[i].renderView->destroyPacket(packet.views[i]);
                std::destroy_at(&packet.views[i]);
            }

            //How long did the frame take
            const double endTime = Platform::getAbsoluteTime();
            elapsedTime = endTime - frameStartTime;
            const double remainingTime = targetTime - elapsedTime;
            //Time left is given back to the OS
            if (remainingTime > 0) {
                const auto remainingMS = static_cast<ULong>(remainingTime * 1000);
                constexpr bool limitFrames = false;
                if (remainingMS > 0 && limitFrames) {
                    Platform::ff_sleep(remainingMS - 1);
                }

                frameCount++;
            }

            //Handle input at the end
            //Must be update -> processInputs or key state wont be tracked correctly
            inputSystem->update(deltaTime);
            platform.processInputs();

            //Update last time
            lastTime = currentTime;
        }
    }

    bIsRunning = false;
}

void Engine::resize(const unsigned short newWidth, const unsigned short newHeight) {
    masterRenderSystem.onResize(newWidth, newHeight);
}

bool Engine::update(float deltaTime) {
    return true;
}

unsigned int Engine::getWindowAttachmentCount() const {
    return masterRenderSystem.getWindowAttachmentCount();
}

Engine::Engine(const GameInstance& instance)
{
    if (!initializeMemory()) throw;

    Logger::initializeFile(logHandler);

    gameInstance = instance;
    gameInstance.state = FF_Memory::ff_allocate<BaseGameState>(GAME, instance.memoryRequirement);

    //Register components
    ComponentRegister<Transform>::registerComponent();
    ComponentRegister<Mesh>::registerComponent();
    ComponentRegister<Camera>::registerComponent();
    ComponentRegister<UITextContainer>::registerComponent();
}

bool Engine::initializeMemory() {
    platform.setPlatform();

    MemoryConfig config{};
    config.totalAllocationSize = GIBIBYTES(1);
    if (!FF_Memory::initialize(config)) {
        Logger::logError("Failed to initialize memory!");
        return false;
    }

    return true;
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

    //Initialize UUID seed
    UUIDUtils::setSeed(101);

    //Initialize event and input systems
    engineEventsSystem.initialize();
    inputSystem->initialize(&engineEventsSystem);

    //Initialize platform class
    if (!platform.initialize(gameInstance.config.appName, gameInstance.config.startingX,
        gameInstance.config.startingY, gameInstance.config.startingWidth, gameInstance.config.startingHeight, inputSystem)) {

        Logger::logFatal("The platform failed to initialize!");
        return;
    }

    //Initialize the resource system
    if (!resourceSystem.initialize("Assets", 32)) {
        Logger::logFatal("Failed to initialize the resource system!");
        return;
    }

    //Start renderer
    if (!masterRenderSystem.initialize(gameInstance.config.appName, platform, gameInstance, resourceSystem)) {
        Logger::logFatal("Failed to initialize the render system!");
        return;
    }

    //Setup configs
    engine->boot();

    //Initialize multithreading
    const bool multithreadRenderer = masterRenderSystem.isRenderSystemMultithreaded();
    int threadCount = platform.getProcessorCount() - 1;
    if (threadCount < 1) {
        Logger::logFatal("Platform reported " + toString(threadCount) + " extra threads. At least 1 extra thread is required for this engine.");
        return;
    }
    Logger::logDebug("Extra threads available: " + toString(threadCount));

    if (threadCount > MAX_THREAD_COUNT) {
        Logger::logDebug("Extra threads will be capped to " + toString(MAX_THREAD_COUNT) + ".");
        threadCount = MAX_THREAD_COUNT;
    }

    unsigned int threadTypes[15]{};
    for (unsigned int& threadType : threadTypes) {
        threadType = GENERAL_JOB;
    }

    if (threadCount == 1 || !multithreadRenderer) {
        threadTypes[0] |= (GPU_JOB | RESOURCE_LOAD_JOB);
    } else if (threadCount == 2) {
        threadTypes[0] |= GPU_JOB;
        threadTypes[1] |= RESOURCE_LOAD_JOB;
    } else {
        threadTypes[0] = GPU_JOB;
        threadTypes[1] = RESOURCE_LOAD_JOB;
    }

    if (!jobSystem.initialize(threadCount, threadTypes)) {
        Logger::logFatal("Failed to initialize the job system.");
        return;
    }

    //Start ECS system
    ECSSystem.initialize();

    //Start camera system
    CameraSystemConfig cameraConfig{};
    cameraConfig.maxCameraCount = 16; //NOTE: THIS DOES NOTHING.
    if (!masterRenderSystem.initializeCameraSystem(cameraConfig, &ECSSystem)) {
        Logger::logFatal("Failed to initialize the camera system!");
        return;
    }

    //Start texture system
    if (!masterRenderSystem.initializeTextureSystem(65536, &resourceSystem)) {
        Logger::logFatal("Failed to initialize the texture system!");
        return;
    }

    //Start the shader system
    if (!masterRenderSystem.initializeShaderSystem(ShaderSystemConfig{1024, 128, 31, 31}, resourceSystem)) {
        Logger::logFatal("Failed to initialize the shader system!");
    }

    //Start render views
    RenderViewSystemConfig renderViewConfig{};
    renderViewConfig.maxViewCount = 251;
    if (!masterRenderSystem.initializeRenderViewSystem(renderViewConfig)) {
        Logger::logFatal("Failed to initialize the render view system!");
        return;
    }

    unsigned int viewCount = gameInstance.config.renderViews.getLength();
    for (unsigned int i = 0; i < viewCount; i++) {
        RenderViewConfig& viewConfig = gameInstance.config.renderViews[i];
        if (!masterRenderSystem.createRenderView(viewConfig)) {
            Logger::logFatal("Failed to create the render view " + viewConfig.name + "!");
            return;
        }
    }
    gameInstance.config.renderViews.shutdown();

    //Start material system
    if (!masterRenderSystem.initializeMaterialSystem(MaterialSystemConfig{4096}, &resourceSystem)) {
        Logger::logFatal("Failed to initialize the texture system!");
        return;
    }

    //Start geometry system
    if (!masterRenderSystem.initializeGeometrySystem(4096, &resourceSystem)) {
        Logger::logFatal("Failed to initialize the geometry system!");
        return;
    }

    //Start Font System
    if (!masterRenderSystem.initializeFontSystem(gameInstance.config.fontConfig)) {
        Logger::logFatal("Failed to initialize the font system!");
        return;
    }

    startup();
}

Engine::~Engine() {
    bIsRunning = false;
    engine = nullptr;

    frameAllocator.shutdown();

    FF_Memory::ff_free_recursive<BitmapFontConfig>(gameInstance.config.fontConfig.bitmapFontConfigs, ARRAY, gameInstance.config.fontConfig.defaultBitmapFontCount);
    FF_Memory::ff_free_recursive<SystemFontConfig>(gameInstance.config.fontConfig.systemFontConfigs, ARRAY, gameInstance.config.fontConfig.defaultSystemFontCount);

    ECSSystem.shutdown();

    gameInstance.shutdown();

    jobSystem.shutdown();

    masterRenderSystem.shutdown();

    engineEventsSystem.shutdown();

    if (inputSystem) {
        FF_Memory::ff_free<IInputSystem>(inputSystem, GAME, inputSystem->getMemorySize());
        inputSystem = nullptr;
    }

    resourceSystem.shutdown();

    MeshUtils::shutdownIdTracker();

    platform.shutdown();

    Logger::logInfo(FF_Memory::getMemoryUsage());

    //THIS MUST ALWAYS SHUTDOWN LAST!
    FF_Memory::shutdown();
    //cleanup logger after memory shutdown so memory errors output to log file.
    Logger::cleanup();
}

void Engine::setEngineRef(Engine& derivedEngine) {
    if (engine != nullptr) return;

    engine = &derivedEngine;
}
