//
// Created by cmorg on 7/1/2026.
//

#include "Engine.h"

#include "../Library/Logger.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Library/GeometryUtils.h"

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
            const unsigned int meshCount = meshes.getLength();
            if (meshCount > 0) {
                packet.geometries.initialize();

                const Quat rotation = getQuatFromAxisAngle({0, 1, 0}, 0.5f * static_cast<float>(deltaTime), false);
                TransformUtils::addRotation(meshes[0].transform, rotation);

                if (meshCount > 1) {
                    TransformUtils::addRotation(meshes[1].transform, rotation);
                }
                if (meshCount > 2) {
                    TransformUtils::addRotation(meshes[2].transform, rotation);
                }

                for (unsigned int i = 0; i < meshCount; i++) {
                    Mesh& mesh = meshes[i];
                    for (unsigned int j = 0; j < mesh.geometryCount; j++) {
                        GeometryRenderData data{};
                        data.geometry = mesh.geometries[j];
                        data.model = TransformUtils::getWorldPos(mesh.transform);
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

    //Temp code
    meshes.initialize(10);
    Mesh cubeMesh{};
    cubeMesh.geometryCount = 1;
    cubeMesh.geometries.initialize(cubeMesh.geometryCount);
    GeometryConfig cubeConfig = masterRenderSystem.generateCubeConfig(10, 10, 10, 1, 1, "Test_Cube_1", "MaterialTemplate");
    GeometryUtils::generateTangents(cubeConfig.vertexCount, static_cast<Vertex3d *>(cubeConfig.vertices), cubeConfig.indexCount, static_cast<unsigned int *>(cubeConfig.indices));
    cubeMesh.geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig, true));
    cubeMesh.transform = Transform{};
    meshes.push(std::move(cubeMesh));
    masterRenderSystem.destroyGeometryConfig(&cubeConfig);

    Mesh cubeMesh2{};
    cubeMesh2.geometryCount = 1;
    cubeMesh2.geometries.initialize(cubeMesh2.geometryCount);
    GeometryConfig cubeConfig2 = masterRenderSystem.generateCubeConfig(5, 5, 5, 1, 1, "Test_Cube_2", "MaterialTemplate");
    GeometryUtils::generateTangents(cubeConfig2.vertexCount, static_cast<Vertex3d *>(cubeConfig2.vertices), cubeConfig2.indexCount, static_cast<unsigned int *>(cubeConfig2.indices));
    cubeMesh2.geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig2, true));
    cubeMesh2.transform = TransformUtils::createTransform(Vector3f{10, 0, 1});
    cubeMesh2.transform.parent = &meshes[0].transform;
    meshes.push(std::move(cubeMesh2));
    masterRenderSystem.destroyGeometryConfig(&cubeConfig2);

    Mesh cubeMesh3{};
    cubeMesh3.geometryCount = 1;
    cubeMesh3.geometries.initialize(cubeMesh3.geometryCount);
    GeometryConfig cubeConfig3 = masterRenderSystem.generateCubeConfig(2, 2, 2, 1, 1, "Test_Cube_3", "MaterialTemplate");
    GeometryUtils::generateTangents(cubeConfig3.vertexCount, static_cast<Vertex3d *>(cubeConfig3.vertices), cubeConfig3.indexCount, static_cast<unsigned int *>(cubeConfig3.indices));
    cubeMesh3.geometries.push(&masterRenderSystem.acquireGeometry(cubeConfig3, true));
    cubeMesh3.transform = TransformUtils::createTransform(Vector3f{5, 0, 1});
    cubeMesh3.transform.parent = &meshes[1].transform;
    meshes.push(std::move(cubeMesh3));
    masterRenderSystem.destroyGeometryConfig(&cubeConfig3);

    //Ui geo
    GeometryConfig configUI{};
    configUI.vertexSize = sizeof(Vertex2d);
    configUI.vertexCount = 4;
    configUI.indexSize = sizeof(unsigned int);
    configUI.indexCount = 6;
    configUI.materialName = "GenericUI";
    configUI.name = "test ui geometry";

    constexpr float w = 512;
    constexpr float h = 256;
    Vertex2d uiVerts[4];
    uiVerts[0].position.x = 0;
    uiVerts[0].position.y = 0;
    uiVerts[0].textureCoordinate.x = 0;
    uiVerts[0].textureCoordinate.y = 0;
    uiVerts[1].position.x = w;
    uiVerts[1].position.y = h;
    uiVerts[1].textureCoordinate.x = 1;
    uiVerts[1].textureCoordinate.y = 1;
    uiVerts[2].position.x = 0;
    uiVerts[2].position.y = h;
    uiVerts[2].textureCoordinate.x = 0;
    uiVerts[2].textureCoordinate.y = 1;
    uiVerts[3].position.x = w;
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
    bIsRunning = false;
    engine = nullptr;

    for (Mesh& mesh : meshes) {
        mesh.geometries.shutdown();
    }
    meshes.shutdown();

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
