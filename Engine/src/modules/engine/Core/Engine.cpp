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
#include "src/modules/engine/ECS/Engine_ECS_Systems/UITextContainerUtils.h"
#include "src/modules/engine/Library/GeometryUtils.h"
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
    inputSystem->subscribeToEngineEvent(QUIT, [this](const EngineInputContext context) {quit();}, "Static.quit");
    inputSystem->subscribeToEngineEvent(RESIZED, [this](const EngineInputContext context) {resize(context.mouseX, context.mouseY);}, "Static.resize");
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
    unsigned char averageCount = 0;
    double msTimes[AVERAGE_COUNT]{};
    double msAverage = 0;
    int frames = 0;
    double accumulatedFrames = 0;
    double fps = 0;

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
            DynamicArray<unsigned int>& basicEntities = EntityComponentSystem::getAllEntitiesOfType("Basic_Entity");
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

            UIPacketData uiPacket{};
            DynamicArray<unsigned int> uiMeshes{0};
            DynamicArray<UIText*> uiTexts{0};
            DynamicArray<unsigned int>& uiEntities = EntityComponentSystem::getAllEntitiesOfType("Basic_UI");
            for (unsigned int i = 0; i < uiEntities.getLength(); i++) {
                const Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(uiEntities[i]);
                if (mesh.generation != INVALID_ID_U8) {
                    uiMeshes.push(uiEntities[i]);
                }

                //Check if the ui entity has text components
                const auto textContainer = EntityComponentSystem::getComponent<UITextContainer>(uiEntities[i]);
                if (textContainer != nullptr) {
                    for (UIText* text : textContainer->texts) {
                        uiTexts.push(text);
                    }
                }

            }
            uiPacket.meshData.meshCount = uiMeshes.getLength();
            uiPacket.meshData.meshes = uiMeshes.getData(); //All of these entities have meshes
            uiPacket.textCount = uiTexts.getLength();
            uiPacket.texts = uiTexts.getData();
            if (!masterRenderSystem.buildPacket(masterRenderSystem.getRenderView("Fox_Fire_UI_View"), &uiPacket, packet.views[2])) {
                Logger::logError("Failed to build ui packet");
                return;
            }
            //end temp code

            //Update ui text
            Camera& worldCamera = *EntityComponentSystem::getComponent<Camera>(getDefaultCamera());
            Vector3f position = CameraUtils::getPosition(worldCamera);
            Vector3f rotation = CameraUtils::getRotation(worldCamera);

            //Get Mouse State
            bool leftDown = inputSystem->isButtonDown(BUTTON_LEFT);
            bool rightDown = inputSystem->isButtonDown(BUTTON_RIGHT);
            int mouseX = 0;
            int mouseY = 0;
            inputSystem->getMousePosition(mouseX, mouseY);

            //Convert to NDC
            float ndcMouseX = convertFloatRange(static_cast<float>(mouseX), 0, width, -1, 1);
            float ndcMouseY = convertFloatRange(static_cast<float>(mouseY), 0, height, -1, 1);

            //Calculate frame average
            double frameMS = elapsedTime * 1000;
            msTimes[averageCount] = frameMS;
            if (averageCount == AVERAGE_COUNT - 1) {
                for (double msTime : msTimes) {
                    msAverage += msTime;
                }

                msAverage /= AVERAGE_COUNT;
            }

            averageCount++;
            averageCount %= AVERAGE_COUNT;

            //Calculate fps
            accumulatedFrames += frameMS;
            if (accumulatedFrames > 1000) {
                fps = frames;
                accumulatedFrames -= 1000;
                frames = 0;
            }

            //Display frame data on UI Text.
            String display = "-----Debug Data-----\nFPS: " + toString(fps, 1) + "(" + toString(msAverage, 1) + "ms)\n" +
                    "POS: [" + vector3fToString(position, 3) + "]\n" +
                        "ROT: [" + vector3fToString(rotation, 3) + "]\n" +
                            "Mouse: X=" + toString(mouseX, 0) + " Y=" + toString(mouseY, 0) + " L=" + toString(leftDown) + " R=" + toString(rightDown) + "\n" +
            "NDC: X=" + toString(ndcMouseX, 6) + " Y=" + toString(ndcMouseY, 6) + "\n" +
                "Hovered: ";
            display += hoveredObjectId == INVALID_ID_U32 ? "None" : toString(hoveredObjectId) + "\n";

            UITextContainer* container = EntityComponentSystem::getComponent<UITextContainer>(uiEntities[0]);
            UIText* text = container->texts[0];
            text->setText(display);

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

            uiPacket.meshData.meshes = nullptr;
            uiEntities.shutdown();
            FF_Memory::ff_free<DynamicArray<unsigned int>>(&uiEntities, DYNAMIC_ARRAY);

            masterRenderSystem.cleanupSkybox(packet);

            //How long did the frame take
            const double endTime = Platform::getAbsoluteTime();
            elapsedTime = endTime - frameStartTime;
            const double remainingTime = targetTime - elapsedTime;
            //Time left is given back to the OS
            if (remainingTime > 0) {
                const auto remainingMS = static_cast<ULong>(remainingTime * 1000);
                constexpr bool limitFrames = false;
                if (remainingMS > 0 && limitFrames) {
                    platform.ff_sleep(remainingMS - 1);
                }

                frameCount++;
            }

            frames++;

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
    if (width != newWidth || height != newHeight) {
        width = newWidth;
        height = newHeight;

        Logger::logDebug("Resized window: " + toString(width) + "x" + toString(height));

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

        auto& uiEntites = ECSSystem.getAllEntitiesOfType("Basic_UI");
        UITextContainer* container = ECSSystem.getComponent<UITextContainer>(uiEntites[0]);
        //Move the debug ui text to the bottom of the screen.
        container->texts[0]->setPosition({20, height - 150.0f, 0});

        uiEntites.shutdown();
        FF_Memory::ff_free<DynamicArray<unsigned int>>(&uiEntites, DYNAMIC_ARRAY);
    }
}

bool Engine::update(float deltaTime) {
    return true;
}

bool Engine::render(float deltaTime) {
    return true;
}

bool Engine::createRenderView(const RenderViewConfig &config) {
    if (!masterRenderSystem.createRenderView(config)) {
        Logger::logFatal("Failed to create world render view.");
        return false;
    }

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

    //Register components
    ComponentRegister<Transform>::registerComponent();
    ComponentRegister<Mesh>::registerComponent();
    ComponentRegister<Camera>::registerComponent();
    ComponentRegister<UITextContainer>::registerComponent();

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

    RenderViewConfig worldConfig{};
    worldConfig.type = RENDER_VIEW_WORLD;
    worldConfig.width = 0;
    worldConfig.height = 0;
    worldConfig.name = "Fox_Fire_World_View";
    worldConfig.renderpassCount = 1;
    RenderpassConfig passConfigs[1]{};
    passConfigs[0].name = "Fox_Fire_World_Renderpass";
    passConfigs[0].renderArea = {0, 0, 1280, 720};
    passConfigs[0].clearColor = {0, 0, 0.2f, 1};
    passConfigs[0].clearFlags = RENDERPASS_CLEAR_DEPTH | RENDERPASS_CLEAR_STENCIL;
    passConfigs[0].depth = 1;
    passConfigs[0].stencil = 0;
    RenderTargetAttachmentConfig targetAttachments[2]{};
    targetAttachments[0].type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    targetAttachments[0].source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    targetAttachments[0].loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_LOAD;
    targetAttachments[0].storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    targetAttachments[0].bPresentAfter = false;
    targetAttachments[1].type = RENDER_TARGET_ATTACHMENT_TYPE_DEPTH;
    targetAttachments[1].source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    targetAttachments[1].loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_DONT_CARE;
    targetAttachments[1].storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    targetAttachments[1].bPresentAfter = false;
    passConfigs[0].targetConfig.attachmentCount = 2;
    passConfigs[0].targetConfig.attachments = targetAttachments;
    passConfigs[0].renderTargetCount = getWindowAttachmentCount();
    worldConfig.renderpasses = passConfigs;
    worldConfig.viewSource = RENDER_VIEW_MATRIX_SOURCE_SCENE;
    if (!createRenderView(worldConfig)) return;

    RenderViewConfig uiConfig{};
    uiConfig.type = RENDER_VIEW_UI;
    uiConfig.width = 0;
    uiConfig.height = 0;
    uiConfig.name = "Fox_Fire_UI_View";
    uiConfig.renderpassCount = 1;
    RenderpassConfig passConfigsUI[1]{};
    passConfigsUI[0].name = "Fox_Fire_UI_Renderpass";
    passConfigsUI[0].renderArea = {0, 0, 1280, 720};
    passConfigsUI[0].clearColor = {0, 0, 0.2f, 1};
    passConfigsUI[0].clearFlags = RENDERPASS_CLEAR_NONE;
    passConfigsUI[0].depth = 1;
    passConfigsUI[0].stencil = 0;
    RenderTargetAttachmentConfig targetAttachmentUI{};
    targetAttachmentUI.type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    targetAttachmentUI.source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    targetAttachmentUI.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_LOAD;
    targetAttachmentUI.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    targetAttachmentUI.bPresentAfter = true;
    passConfigsUI[0].targetConfig.attachmentCount = 1;
    passConfigsUI[0].targetConfig.attachments = &targetAttachmentUI;
    passConfigsUI[0].renderTargetCount = getWindowAttachmentCount();
    uiConfig.renderpasses = passConfigsUI;
    uiConfig.viewSource = RENDER_VIEW_MATRIX_SOURCE_SCENE;
    if (!createRenderView(uiConfig)) return;

    RenderViewConfig skyboxConfig{};
    skyboxConfig.type = RENDER_VIEW_SKYBOX;
    skyboxConfig.width = 0;
    skyboxConfig.height = 0;
    skyboxConfig.name = "Fox_Fire_Skybox_View";
    skyboxConfig.renderpassCount = 1;
    RenderpassConfig passConfigsSkybox[1]{};
    passConfigsSkybox[0].name = "Fox_Fire_Skybox_Renderpass";
    passConfigsSkybox[0].renderArea = {0, 0, 1280, 720};
    passConfigsSkybox[0].clearColor = {0, 0, 0.2f, 1};
    passConfigsSkybox[0].clearFlags = RENDERPASS_CLEAR_COLOR;
    passConfigsSkybox[0].depth = 1;
    passConfigsSkybox[0].stencil = 0;
    RenderTargetAttachmentConfig skyboxTargetAttachment{};
    skyboxTargetAttachment.type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    skyboxTargetAttachment.source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    skyboxTargetAttachment.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_DONT_CARE;
    skyboxTargetAttachment.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    skyboxTargetAttachment.bPresentAfter = false;
    passConfigsSkybox[0].targetConfig.attachmentCount = 1;
    passConfigsSkybox[0].targetConfig.attachments = &skyboxTargetAttachment;
    passConfigsSkybox[0].renderTargetCount = getWindowAttachmentCount();
    skyboxConfig.renderpasses = passConfigsSkybox;
    skyboxConfig.viewSource = RENDER_VIEW_MATRIX_SOURCE_SCENE;
    if (!createRenderView(skyboxConfig)) return;

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
    FontSystemConfig fontConfig{};
    fontConfig.bAutoRelease = false;
    fontConfig.defaultBitmapFontCount = 1;
    fontConfig.defaultSystemFontCount = 1;
    fontConfig.maxBitmapFonts = 101;
    fontConfig.maxSystemFonts = 101;

    BitmapFontConfig bitmapFontConfig{};
    bitmapFontConfig.name = "Ubuntu-px24";
    bitmapFontConfig.resourceName = "Ubuntu-px24";
    bitmapFontConfig.size = 48;
    fontConfig.bitmapFontConfigs = &bitmapFontConfig;

    SystemFontConfig systemFontConfig{};
    systemFontConfig.defaultSize = 24;
    systemFontConfig.name = "Ubuntu";
    systemFontConfig.resourceName = "Ubuntu";
    fontConfig.systemFontConfigs = &systemFontConfig;


    if (!masterRenderSystem.initializeFontSystem(fontConfig, &resourceSystem)) {
        Logger::logFatal("Failed to initialize the font system!");
        return;
    }

    masterRenderSystem.initializeSkybox();

    //Temp code
    const unsigned int cube1 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh = EntityComponentSystem::getComponent<Mesh>(cube1);
    cubeMesh->geometryCount = 1;
    cubeMesh->geometries.initialize(cubeMesh->geometryCount);
    GeometryConfig cubeConfig = GeometrySystem::generateCubeConfig(10, 10, 10, 1, 1, "Test_Cube_1", "MaterialTemplate");

    cubeMesh->geometries.push(&GeometrySystem::acquireGeometry(cubeConfig, true));
    cubeMesh->generation = 0;
    GeometryUtils::destroyConfig(&cubeConfig);

    const unsigned int cube2 = ECSSystem.createEntity("Basic_Entity");
    Mesh* cubeMesh2 = EntityComponentSystem::getComponent<Mesh>(cube2);
    cubeMesh2->geometryCount = 1;
    cubeMesh2->geometries.initialize(cubeMesh2->geometryCount);
    GeometryConfig cubeConfig2 = GeometrySystem::generateCubeConfig(5, 5, 5, 1, 1, "Test_Cube_2", "MaterialTemplate");
    cubeMesh2->geometries.push(&GeometrySystem::acquireGeometry(cubeConfig2, true));
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
    GeometryConfig cubeConfig3 = GeometrySystem::generateCubeConfig(2, 2, 2, 1, 1, "Test_Cube_3", "MaterialTemplate");
    cubeMesh3->geometries.push(&GeometrySystem::acquireGeometry(cubeConfig3, true));
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

    ui1Mesh->geometries.push(&GeometrySystem::acquireGeometry(configUI, true));
    ui1Mesh->generation = 0;

    auto textContainer = EntityComponentSystem::addComponent<UITextContainer>(ui1);
    UIText* testText = UITextContainerUtils::addText(*textContainer, UI_TEXT_TYPE_BITMAP, "Ubuntu-px24", 12, "The following \t3\t lines represent the truth of the world:\nYou\nAre\nGullible");
    if (testText == nullptr) return;

    UIText* testSystemText = UITextContainerUtils::addText(*textContainer, UI_TEXT_TYPE_SYSTEM, "Ubuntu-Regular", 24, "Do not be fooled by\t[REDACTED]\t\nYou are not gullible!\nYou are just an ばか.");
    if (testSystemText == nullptr) return;

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

    gameInstance.shutdown();

    jobSystem.shutdown();

    masterRenderSystem.shutdown();

    engineEventsSystem.shutdown();

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
