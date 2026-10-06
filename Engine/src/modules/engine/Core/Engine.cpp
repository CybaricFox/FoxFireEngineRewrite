//
// Created by cmorg on 7/1/2026.
//

#include "Engine.h"

#include "../Library/Logger.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/MeshUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Library/GeometryUtils.h"
#include "src/modules/engine/Library/JsonHandler.h"
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
    inputSystem->subscribeToEngineEvent(QUIT, [this](const EngineInputContext context) {quit();}, "Static.quit");
    inputSystem->subscribeToEngineEvent(RESIZED, [this](const EngineInputContext context) {resize(context.mouseX, context.mouseY);}, "Static.resize");

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
            clock.update();
            const double currentTime = clock.getElapsedTime();
            const double deltaTime = currentTime - lastTime;
            const double frameStartTime = Platform::getAbsoluteTime();

            //Updates jobs
            jobSystem.update();

            if (!engine->update(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game update tick failed!");
                bIsRunning = false;
            }

            if (!render(static_cast<float>(deltaTime))) {
                Logger::logFatal("Game render tick failed!");
                bIsRunning = false;
            }

            //temp code
            const unsigned int meshCount = ECSSystem.getEntityCount("Basic_Entity");
            if (meshCount > 0) {
                const Quat rotation = getQuatFromAxisAngle({0, 1, 0}, 0.5f * static_cast<float>(deltaTime), false);
                TransformUtils::addRotation(*EntityComponentSystem::getComponent<Transform>(1), rotation);

                if (meshCount > 1) {
                    TransformUtils::addRotation(*EntityComponentSystem::getComponent<Transform>(2), rotation);
                }
                if (meshCount > 2) {
                    TransformUtils::addRotation(*EntityComponentSystem::getComponent<Transform>(3), rotation);
                }
            }

            RenderPacket packet{};
            packet.deltaTime = static_cast<float>(deltaTime);
            packet.viewCount = 3;
            RenderViewPacket views[3]{};
            packet.views = views;

            masterRenderSystem.buildSkybox(packet);

            MeshPacketData worldMeshData{};
            DynamicArray<unsigned int> worldMeshes{0};
            DynamicArray<unsigned int>& basicEntities = ECSSystem.getAllEntitiesOfType("Basic_Entity");
            for (unsigned int i = 0; i < meshCount; i++) {
                const Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(basicEntities[i]);
                if (mesh.generation != INVALID_ID_U8) {
                    worldMeshes.push(basicEntities[i]);
                }
            }
            worldMeshData.meshCount = worldMeshes.getLength();
            worldMeshData.meshes = worldMeshes.getData(); //All of these entities have meshes
            if (!masterRenderSystem.buildPacket(masterRenderSystem.getRenderView("Fox_Fire_World_View"), &worldMeshData, packet.views[1])) {
                Logger::logError("Failed to build world packet");
                return;
            }

            MeshPacketData uiMeshData{};
            DynamicArray<unsigned int> uiMeshes{0};
            DynamicArray<unsigned int>& uiEntities = ECSSystem.getAllEntitiesOfType("Basic_UI");
            for (unsigned int i = 0; i < uiEntities.getLength(); i++) {
                const Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(uiEntities[i]);
                if (mesh.generation != INVALID_ID_U8) {
                    uiMeshes.push(uiEntities[i]);
                }
            }
            uiMeshData.meshCount = uiMeshes.getLength();
            uiMeshData.meshes = uiMeshes.getData(); //All of these entities have meshes
            if (!masterRenderSystem.buildPacket(masterRenderSystem.getRenderView("Fox_Fire_UI_View"), &uiMeshData, packet.views[2])) {
                Logger::logError("Failed to build ui packet");
                return;
            }
            //end temp code

            if (!masterRenderSystem.drawFrame(packet)) {
                Logger::logFatal("Failed to draw frame!");
                bIsRunning = false;
            }

            //Cleanup Packet
            for (unsigned int i = 0; i < packet.viewCount; i++) {
                packet.views[i].renderView->destroyPacket(packet.views[i]);
            }

            worldMeshData.meshes = nullptr;
            basicEntities.shutdown();
            FF_Memory::ff_free<DynamicArray<unsigned int>>(&basicEntities, DYNAMIC_ARRAY);

            uiMeshData.meshes = nullptr;
            uiEntities.shutdown();
            FF_Memory::ff_free<DynamicArray<unsigned int>>(&uiEntities, DYNAMIC_ARRAY);

            masterRenderSystem.cleanupSkybox(packet);

            //How long did the frame take
            const double endTime = Platform::getAbsoluteTime();
            const double elapsedTime = endTime - frameStartTime;
            runTime += elapsedTime;
            const double remainingTime = targetTime - elapsedTime;
            //Time left is given back to the OS
            if (remainingTime > 0) {
                const auto remainingMS = static_cast<unsigned long>(remainingTime * 1000);
                constexpr bool limitFrames = false;
                if (remainingMS > 0 && limitFrames) {
                    platform.ff_sleep(remainingMS - 1);
                }

                frameCount++;
            }
            //Handle input at the end
            //Must be update -> processInputs or key state wont be tracked correctly
            inputSystem->update(deltaTime);
            platform.processInputs();

            //Update last time
            lastTime = currentTime;
            if (deltaCount >= 1) {
                Logger::logInfo("FPS: " + toString(fps));
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

void Engine::createRenderView(const RenderViewConfig &config) {
    if (!masterRenderSystem.createRenderView(config)) {
        Logger::logFatal("Failed to create world render view.");
    }
}

Engine::Engine(const GameInstance& instance)
{
    if (!initializeMemory()) throw;

    Logger::initializeFile(logHandler);
    gameInstance = instance;
    gameInstance.state = FF_Memory::ff_allocate<BaseGameState>(GAME, instance.memoryRequirement);
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
    MeshUtils::setResourceSystemRef(&resourceSystem);

    //Start renderer
    if (!masterRenderSystem.initialize(gameInstance.config.appName, platform, gameInstance, resourceSystem)) {
        Logger::logFatal("Failed to initialize the render system!");
        return;
    }

    //Initialize multithreading
    bool multithreadRenderer = masterRenderSystem.isRenderSystemMultithreaded();
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

    //Start render views
    RenderViewSystemConfig renderViewConfig{};
    renderViewConfig.maxViewCount = 251;
    if (!masterRenderSystem.initializeRenderViewSystem(renderViewConfig)) {
        Logger::logFatal("Failed to initialize the render view system!");
        return;
    }

    masterRenderSystem.initializeSkybox();

    //Temp code
    const unsigned int cube1 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh = EntityComponentSystem::getComponent<Mesh>(cube1);
    cubeMesh->geometryCount = 1;
    cubeMesh->geometries.initialize(cubeMesh->geometryCount);
    GeometryConfig cubeConfig = masterRenderSystem.generateCubeConfig(10, 10, 10, 1, 1, "Test_Cube_1", "MaterialTemplate");

    cubeMesh->geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig, true));
    cubeMesh->generation = 0;
    GeometryUtils::destroyConfig(&cubeConfig);

    const unsigned int cube2 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh2 = EntityComponentSystem::getComponent<Mesh>(cube2);
    cubeMesh2->geometryCount = 1;
    cubeMesh2->geometries.initialize(cubeMesh2->geometryCount);
    GeometryConfig cubeConfig2 = masterRenderSystem.generateCubeConfig(5, 5, 5, 1, 1, "Test_Cube_2", "MaterialTemplate");
    cubeMesh2->geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig2, true));
    const auto cube2Transform = EntityComponentSystem::getComponent<Transform>(cube2);
    cube2Transform->position = Vector3f{10, 0, 1};
    cube2Transform->parent = cube1;
    cube2Transform->bIsDirty = true;
    cubeMesh2->generation = 0;
    GeometryUtils::destroyConfig(&cubeConfig2);

    const unsigned int cube3 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh3 = EntityComponentSystem::getComponent<Mesh>(cube3);
    cubeMesh3->geometryCount = 1;
    cubeMesh3->geometries.initialize(cubeMesh3->geometryCount);
    GeometryConfig cubeConfig3 = masterRenderSystem.generateCubeConfig(2, 2, 2, 1, 1, "Test_Cube_3", "MaterialTemplate");
    cubeMesh3->geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig3, true));
    const auto cube3Transform = EntityComponentSystem::getComponent<Transform>(cube3);
    cube3Transform->position = Vector3f{5, 0, 1};
    cube3Transform->parent = cube2;
    cube3Transform->bIsDirty = true;
    cubeMesh3->generation = 0;
    GeometryUtils::destroyConfig(&cubeConfig3);

    const unsigned int maxwell = ECSSystem.createEntity("Basic_Entity");

    const auto maxwellTransform = EntityComponentSystem::getComponent<Transform>(maxwell);
    maxwellTransform->position = Vector3f{15, 0, 1};
    maxwellTransform->scale = Vector3f{10, 10, 10};
    maxwellTransform->bIsDirty = true;

    //Ui geo
    const unsigned int ui1 = ECSSystem.createEntity("Basic_UI");
    Mesh* ui1Mesh = EntityComponentSystem::getComponent<Mesh>(ui1);
    ui1Mesh->geometryCount = 1;
    ui1Mesh->geometries.initialize(ui1Mesh->geometryCount);

    GeometryConfig configUI{};
    configUI.vertices.initialize<Vertex2d>(4);
    configUI.indices.initialize<unsigned int>(6);
    configUI.materialName = "GenericUI";
    configUI.name = "test ui geometry";

    constexpr float w = 512;
    constexpr float h = 256;
    auto array = reinterpret_cast<Vertex2d *>(configUI.vertices.getVertex(0));
    array[0].position.x = 0;
    array[0].position.y = 0;
    array[0].textureCoordinate.x = 0;
    array[0].textureCoordinate.y = 0;
    array[1].position.x = w;
    array[1].position.y = h;
    array[1].textureCoordinate.x = 1;
    array[1].textureCoordinate.y = 1;
    array[2].position.x = 0;
    array[2].position.y = h;
    array[2].textureCoordinate.x = 0;
    array[2].textureCoordinate.y = 1;
    array[3].position.x = w;
    array[3].position.y = 0;
    array[3].textureCoordinate.x = 1;
    array[3].textureCoordinate.y = 0;

    const unsigned int uiIndices[6] = {2, 1, 0, 3, 0, 1};
    for (unsigned int i = 0; i < 6; i++) {
        configUI.indices.setIndex(uiIndices[i], i);
    }

    ui1Mesh->geometries[0] = &masterRenderSystem.acquireGeometry(configUI, true);
    ui1Mesh->generation = 0;
    //End temp code

    startup();
}

void Engine::getFramebufferSize(unsigned int& bufferWidth, unsigned int& bufferHeight) const {
    bufferWidth = width;
    bufferHeight = height;
}

Engine::~Engine() {
    bIsRunning = false;
    engine = nullptr;

    ECSSystem.shutdown();

    //Static destruction
    engineEventsSystem.shutdown();

    gameInstance.shutdown();

    jobSystem.shutdown();

    //Destroy resources in opposite order of creation
    geometrySystem = nullptr;
    materialSystem = nullptr;
    textureSystem = nullptr;
    masterRenderSystem.shutdown();

    if (inputSystem) {
        FF_Memory::ff_free<IInputSystem>(inputSystem, GAME, inputSystem->getMemorySize());
        inputSystem = nullptr;
    }

    resourceSystem.shutdown();

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
