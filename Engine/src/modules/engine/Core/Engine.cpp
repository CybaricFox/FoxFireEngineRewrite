//
// Created by cmorg on 7/1/2026.
//

#include "Engine.h"

#include "../Library/Logger.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Library/GeometryUtils.h"
#include "src/modules/engine/Library/JsonHandler.h"

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
    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {masterRenderSystem.changeRenderMode(static_cast<Keys>(context.key));}, "MasterRender.default_render");

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
            const unsigned int meshCount = ECSSystem.getEntityCount("Basic_Entity");
            if (meshCount > 0) {
                packet.geometries.initialize();

                const Quat rotation = getQuatFromAxisAngle({0, 1, 0}, 0.5f * static_cast<float>(deltaTime), false);
                TransformUtils::addRotation(*ECSSystem.getComponent<Transform>(0), rotation);

                if (meshCount > 1) {
                    TransformUtils::addRotation(*ECSSystem.getComponent<Transform>(1), rotation);
                }
                if (meshCount > 2) {
                    TransformUtils::addRotation(*ECSSystem.getComponent<Transform>(2), rotation);
                }

                for (unsigned int i = 0; i < meshCount; i++) {
                    Mesh& mesh = *ECSSystem.getComponent<Mesh>(i);
                    for (unsigned int j = 0; j < mesh.geometryCount; j++) {
                        GeometryRenderData data{};
                        data.geometry = mesh.geometries[j];
                        data.model = TransformUtils::getWorldPos(*mesh.transform);
                        packet.geometries.push(data);
                        packet.geometryCount++;
                    }
                }
            }

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

            if (!packet.geometries.isEmpty()) {
                packet.geometries.shutdown();
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
            platform.processInputs();

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

Engine::Engine(const GameInstance& instance)
{
    if (!initializeMemory()) throw;
    Logger::initializeFile(logHandler);
    gameInstance = instance;
    gameInstance.state = FF_Memory::ff_allocate_class<BaseGameState>(instance.memoryRequirement, GAME);
}

bool Engine::initializeMemory() {
    MemoryConfig config{};
    config.totalAllocationSize = GIBIBYTES(1);
    if (!FF_Memory::initialize(config)) {
        Logger::logError("Failed to initialize memory!");
        return false;
    }

    FF_Memory::trackEngineMemory(sizeof(Engine));

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

    //Start renderer
    if (!masterRenderSystem.initialize(gameInstance.config.appName, platform, gameInstance, width, height, resourceSystem)) {
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

    ECSSystem.initialize();

    //Temp code
    const unsigned int cube1 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh = ECSSystem.getComponent<Mesh>(cube1);
    cubeMesh->geometryCount = 1;
    cubeMesh->geometries.initialize(cubeMesh->geometryCount);
    GeometryConfig cubeConfig = masterRenderSystem.generateCubeConfig(10, 10, 10, 1, 1, "Test_Cube_1", "MaterialTemplate");

    cubeMesh->geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig, true));
    cubeMesh->transform = ECSSystem.getComponent<Transform>(cube1);
    GeometryUtils::destroyConfig(&cubeConfig);

    const unsigned int cube2 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh2 = ECSSystem.getComponent<Mesh>(cube2);
    cubeMesh2->geometryCount = 1;
    cubeMesh2->geometries.initialize(cubeMesh2->geometryCount);
    GeometryConfig cubeConfig2 = masterRenderSystem.generateCubeConfig(5, 5, 5, 1, 1, "Test_Cube_2", "MaterialTemplate");
    cubeMesh2->geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig2, true));
    cubeMesh2->transform = ECSSystem.getComponent<Transform>(cube2);
    cubeMesh2->transform->position = Vector3f{10, 0, 1};
    cubeMesh2->transform->parent = ECSSystem.getComponent<Transform>(0);
    GeometryUtils::destroyConfig(&cubeConfig2);

    const unsigned int cube3 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh3 = ECSSystem.getComponent<Mesh>(cube3);
    cubeMesh3->geometryCount = 1;
    cubeMesh3->geometries.initialize(cubeMesh3->geometryCount);
    GeometryConfig cubeConfig3 = masterRenderSystem.generateCubeConfig(2, 2, 2, 1, 1, "Test_Cube_3", "MaterialTemplate");
    cubeMesh3->geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig3, true));
    cubeMesh3->transform = ECSSystem.getComponent<Transform>(cube3);
    cubeMesh3->transform->position = Vector3f{5, 0, 1};
    cubeMesh3->transform->parent = ECSSystem.getComponent<Transform>(1);
    GeometryUtils::destroyConfig(&cubeConfig3);

    const unsigned int maxwell = ECSSystem.createEntity("Basic_Entity");
    Mesh* maxwellMesh = ECSSystem.getComponent<Mesh>(maxwell);
    Resource maxwellResource{};
    if (!resourceSystem.load("Maxwell", RESOURCE_TYPE_MESH, maxwellResource)) {
        Logger::logFatal("Maxwell? Maxwell?! MAXWELL!!!!!!!");
        return;
    } else {
        GeometryConfig* maxwellConfigs = &(*static_cast<DynamicArray<GeometryConfig>*>(maxwellResource.data))[0];
        maxwellMesh->geometryCount = maxwellResource.dataSize; //Data size in this context is the number of geometries
        maxwellMesh->geometries.initialize(maxwellMesh->geometryCount);
        for (unsigned int i = 0; i < maxwellMesh->geometryCount; i++) {
            GeometryConfig* currentConfig = &maxwellConfigs[i];
            maxwellMesh->geometries.push(&masterRenderSystem.acquireGeometry(maxwellConfigs[i], true));
        }
        maxwellMesh->transform = ECSSystem.getComponent<Transform>(maxwell);
        maxwellMesh->transform->position = Vector3f{15, 0, 1};
        maxwellMesh->transform->scale = Vector3f{10, 10, 10};
        maxwellMesh->transform->bIsDirty = true;
        resourceSystem.unload(maxwellResource);
    }

    //Ui geo
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

    testUIGeometry = &masterRenderSystem.acquireGeometry(configUI, true);
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

    //Destroy resources in opposite order of creation
    geometrySystem = nullptr;
    materialSystem = nullptr;
    textureSystem = nullptr;
    masterRenderSystem.shutdown();

    if (inputSystem) {
        FF_Memory::ff_free_class<IInputSystem>(inputSystem, inputSystem->getMemorySize(), GAME);
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
