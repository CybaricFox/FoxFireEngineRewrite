//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/ECS/Engine_Components/UITextContainer.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/MeshUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Library/Logger.h"
#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"
#include "src/modules/system/FoxFire_Renders/WorldRenderView.h"

void Game::loadModels() {
    if (!bModelsLoaded) {
        Logger::logDebug("Loading models");
        bModelsLoaded = true;

        const GameState& state = *reinterpret_cast<GameState *>(gameInstance.state);

        if (!MeshUtils::loadMeshFromResource("Maxwell", state.maxwell)) {
            Logger::logError("Failed to load Maxwell.");
        }
    }
}

bool Game::configureRenderViews() {
    gameInstance.config.renderViews.initialize(4);

    //Load render views
    RenderViewConfig& worldConfig = *gameInstance.config.renderViews.emplace();
    worldConfig.type = RENDER_VIEW_WORLD;
    worldConfig.width = 0;
    worldConfig.height = 0;
    worldConfig.name = "Fox_Fire_World_View";
    worldConfig.viewSource = RENDER_VIEW_MATRIX_SOURCE_SCENE;
    worldConfig.renderpasses.initialize(1);
    RenderpassConfig& worldPassConfig = *worldConfig.renderpasses.emplace();
    worldPassConfig.name = "Fox_Fire_World_Renderpass";
    worldPassConfig.renderArea = {0, 0, 1280, 720};
    worldPassConfig.clearColor = {0, 0, 0.2f, 1};
    worldPassConfig.clearFlags = RENDERPASS_CLEAR_DEPTH | RENDERPASS_CLEAR_STENCIL;
    worldPassConfig.depth = 1;
    worldPassConfig.stencil = 0;
    worldPassConfig.targetConfig.attachments.initialize(2);
    RenderTargetAttachmentConfig& worldAttachment1 = *worldPassConfig.targetConfig.attachments.emplace();
    worldAttachment1.type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    worldAttachment1.source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    worldAttachment1.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_LOAD;
    worldAttachment1.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    worldAttachment1.bPresentAfter = false;
    RenderTargetAttachmentConfig& worldAttachment2 = *worldPassConfig.targetConfig.attachments.emplace();
    worldAttachment2.type = RENDER_TARGET_ATTACHMENT_TYPE_DEPTH;
    worldAttachment2.source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    worldAttachment2.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_DONT_CARE;
    worldAttachment2.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    worldAttachment2.bPresentAfter = false;
    worldPassConfig.renderTargetCount = masterRenderSystem.getWindowAttachmentCount();

    RenderViewConfig& uiConfig = *gameInstance.config.renderViews.emplace();
    uiConfig.type = RENDER_VIEW_UI;
    uiConfig.width = 0;
    uiConfig.height = 0;
    uiConfig.name = "Fox_Fire_UI_View";
    uiConfig.viewSource = RENDER_VIEW_MATRIX_SOURCE_SCENE;
    uiConfig.renderpasses.initialize(1);
    RenderpassConfig& uiPassConfig = *uiConfig.renderpasses.emplace();
    uiPassConfig.name = "Fox_Fire_UI_Renderpass";
    uiPassConfig.renderArea = {0, 0, 1280, 720};
    uiPassConfig.clearColor = {0, 0, 0.2f, 1};
    uiPassConfig.clearFlags = RENDERPASS_CLEAR_NONE;
    uiPassConfig.depth = 1;
    uiPassConfig.stencil = 0;
    uiPassConfig.targetConfig.attachments.initialize(1);
    RenderTargetAttachmentConfig& uiAttachment = *uiPassConfig.targetConfig.attachments.emplace();
    uiAttachment.type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    uiAttachment.source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    uiAttachment.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_LOAD;
    uiAttachment.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    uiAttachment.bPresentAfter = true;
    uiPassConfig.renderTargetCount = masterRenderSystem.getWindowAttachmentCount();

    RenderViewConfig& skyboxConfig = *gameInstance.config.renderViews.emplace();
    skyboxConfig.type = RENDER_VIEW_SKYBOX;
    skyboxConfig.width = 0;
    skyboxConfig.height = 0;
    skyboxConfig.name = "Fox_Fire_Skybox_View";
    skyboxConfig.viewSource = RENDER_VIEW_MATRIX_SOURCE_SCENE;
    skyboxConfig.renderpasses.initialize(1);
    RenderpassConfig& skyboxPassConfig = *skyboxConfig.renderpasses.emplace();
    skyboxPassConfig.name = "Fox_Fire_Skybox_Renderpass";
    skyboxPassConfig.renderArea = {0, 0, 1280, 720};
    skyboxPassConfig.clearColor = {0, 0, 0.2f, 1};
    skyboxPassConfig.clearFlags = RENDERPASS_CLEAR_COLOR;
    skyboxPassConfig.depth = 1;
    skyboxPassConfig.stencil = 0;
    skyboxPassConfig.targetConfig.attachments.initialize(1);
    RenderTargetAttachmentConfig& skyboxAttachment = *skyboxPassConfig.targetConfig.attachments.emplace();
    skyboxAttachment.type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    skyboxAttachment.source = RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT;
    skyboxAttachment.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_DONT_CARE;
    skyboxAttachment.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    skyboxAttachment.bPresentAfter = false;
    skyboxPassConfig.renderTargetCount = masterRenderSystem.getWindowAttachmentCount();

    RenderViewConfig& pickConfig = *gameInstance.config.renderViews.emplace();
    pickConfig.type = RENDER_VIEW_PICK;
    pickConfig.width = 0;
    pickConfig.height = 0;
    pickConfig.name = "Fox_Fire_Pick_View";
    pickConfig.viewSource = RENDER_VIEW_MATRIX_SOURCE_SCENE;
    pickConfig.renderpasses.initialize(2);
    RenderpassConfig& pickPassConfig1 = *pickConfig.renderpasses.emplace();
    pickPassConfig1.name = "Fox_Fire_World_Pick_Renderpass";
    pickPassConfig1.renderArea = {0, 0, 1280, 720};
    pickPassConfig1.clearColor = {1, 1, 1, 1};
    pickPassConfig1.clearFlags = RENDERPASS_CLEAR_COLOR | RENDERPASS_CLEAR_DEPTH;
    pickPassConfig1.depth = 1;
    pickPassConfig1.stencil = 0;
    pickPassConfig1.targetConfig.attachments.initialize(1);
    RenderTargetAttachmentConfig& pickAttachment1 = *pickPassConfig1.targetConfig.attachments.emplace();
    pickAttachment1.type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    pickAttachment1.source = RENDER_TARGET_ATTACHMENT_SOURCE_VIEW;
    pickAttachment1.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_DONT_CARE;
    pickAttachment1.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    pickAttachment1.bPresentAfter = false;
    RenderTargetAttachmentConfig& pickAttachment2 = *pickPassConfig1.targetConfig.attachments.emplace();
    pickAttachment2.type = RENDER_TARGET_ATTACHMENT_TYPE_DEPTH;
    pickAttachment2.source = RENDER_TARGET_ATTACHMENT_SOURCE_VIEW;
    pickAttachment2.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_DONT_CARE;
    pickAttachment2.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    pickAttachment2.bPresentAfter = false;
    pickPassConfig1.renderTargetCount = 1;
    RenderpassConfig& pickPassConfig2 = *pickConfig.renderpasses.emplace();
    pickPassConfig2.name = "Fox_Fire_UI_Pick_Renderpass";
    pickPassConfig2.renderArea = {0, 0, 1280, 720};
    pickPassConfig2.clearColor = {1, 1, 1, 1};
    pickPassConfig2.clearFlags = RENDERPASS_CLEAR_NONE;
    pickPassConfig2.depth = 1;
    pickPassConfig2.stencil = 0;
    pickPassConfig2.targetConfig.attachments.initialize(1);
    RenderTargetAttachmentConfig& pick2Attachment1 = *pickPassConfig2.targetConfig.attachments.emplace();
    pick2Attachment1.type = RENDER_TARGET_ATTACHMENT_TYPE_COLOR;
    pick2Attachment1.source = RENDER_TARGET_ATTACHMENT_SOURCE_VIEW;
    pick2Attachment1.loadOperation = RENDER_TARGET_ATTACHMENT_LOAD_OP_LOAD;
    pick2Attachment1.storeOperation = RENDER_TARGET_ATTACHMENT_STORE_OP_STORE;
    pick2Attachment1.bPresentAfter = false;
    pickPassConfig2.renderTargetCount = 1;

    return true;
}

Game::Game(const GameInstance& instance)
    :Engine(instance)
{
    inputSystem = FF_Memory::ff_allocate<FoxFire_InputSystem>(GAME);
}

Game::~Game() {
    GameState& state = *reinterpret_cast<GameState *>(gameInstance.state);

    state.skybox.shutdown();
    state.bitmapText.destroy();
    state.systemText.destroy();

    swapTextureEvent.destroyEvent();
}

void Game::startup() {
    GameState& state = *reinterpret_cast<GameState *>(gameInstance.state);

    //Register events
    swapTextureEvent.registerEvent();
    loadModelsEvent.registerEvent();

    //Assign the camera
    state.worldCamera = CameraSystem::getDefaultCamera();
    CameraUtils::setPosition(*EntityComponentSystem::getComponent<Camera>(state.worldCamera), {10.5f, 5, 9.5f});

    //Create ui text objects
    if (!state.bitmapText.create(UI_TEXT_TYPE_BITMAP, "Ubuntu-px24", 48, "It says Gullible on the cieling.")) {
        Logger::logError("Failed to create bitmap UI text.");
        return;
    }
    state.bitmapText.setPosition({20, static_cast<float>(gameInstance.config.startingHeight - 150), 0});

    if (!state.systemText.create(UI_TEXT_TYPE_SYSTEM, "Ubuntu-Regular", 24, "BOTTOM TEXT.")) {
        Logger::logError("Failed to create system UI text.");
        return;
    }
    state.systemText.setPosition({50, 250, 0});

    //Create Skybox
    if (!state.skybox.initialize("Maxwell_Skybox")) {
        Logger::logError("Failed to initialize skybox.");
        return;
    }

    //Create World Meshes
    for (unsigned int i = 0; i < 10; i++) {
        state.worldEntities[i] = INVALID_ID_U32;
        state.uiEntities[i] = INVALID_ID_U32;
    }

    state.worldEntities[0] = ECSSystem.createEntity("Basic_Entity");
    Mesh& world1 = *EntityComponentSystem::getComponent<Mesh>(state.worldEntities[0]);
    world1.geometryCount = 1;
    world1.geometries.initialize(world1.geometryCount);
    GeometryConfig geometryConfig = GeometrySystem::generateCubeConfig(10, 10, 10, 1, 1, "Test_Cube_1", "MaterialTemplate");
    world1.geometries.push(&GeometrySystem::acquireGeometry(geometryConfig, true));
    world1.generation = 0;
    GeometryUtils::destroyConfig(&geometryConfig);

    state.worldEntities[1] = ECSSystem.createEntity("Basic_Entity");
    Mesh& world2 = *EntityComponentSystem::getComponent<Mesh>(state.worldEntities[1]);
    world2.geometryCount = 1;
    world2.geometries.initialize(world2.geometryCount);
    GeometryConfig geometryConfig2 = GeometrySystem::generateCubeConfig(5, 5, 5, 1, 1, "Test_Cube_2", "MaterialTemplate");
    world2.geometries.push(&GeometrySystem::acquireGeometry(geometryConfig2, true));
    Transform& world2Transform = *EntityComponentSystem::getComponent<Transform>(state.worldEntities[1]);
    world2Transform.parent = state.worldEntities[0];
    world2Transform.position = Vector3f{10, 0, 1};
    world2Transform.bIsDirty = true;
    world2.generation = 0;
    GeometryUtils::destroyConfig(&geometryConfig2);

    state.worldEntities[2] = ECSSystem.createEntity("Basic_Entity");
    Mesh* world3 = EntityComponentSystem::getComponent<Mesh>(state.worldEntities[2]);
    world3->geometryCount = 1;
    world3->geometries.initialize(world3->geometryCount);
    GeometryConfig geometryConfig3 = GeometrySystem::generateCubeConfig(2, 2, 2, 1, 1, "Test_Cube_3", "MaterialTemplate");
    world3->geometries.push(&GeometrySystem::acquireGeometry(geometryConfig3, true));
    Transform& world3Transform = *EntityComponentSystem::getComponent<Transform>(state.worldEntities[2]);
    world3Transform.position = Vector3f{5, 0, 1};
    world3Transform.parent = state.worldEntities[1];
    world3Transform.bIsDirty = true;
    world3->generation = 0;
    GeometryUtils::destroyConfig(&geometryConfig3);

    state.maxwell = ECSSystem.createEntity("Basic_Entity");

    auto& maxwellTransform = *EntityComponentSystem::getComponent<Transform>(state.maxwell);
    maxwellTransform.position = Vector3f{15, 0, 1};
    maxwellTransform.scale = Vector3f{10, 10, 10};
    maxwellTransform.bIsDirty = true;

    //Create UI Meshes
    state.uiEntities[0] = ECSSystem.createEntity("Basic_UI");
    Mesh& ui1Mesh = *EntityComponentSystem::getComponent<Mesh>(state.uiEntities[0]);
    ui1Mesh.geometryCount = 1;
    ui1Mesh.geometries.initialize(ui1Mesh.geometryCount);

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

    ui1Mesh.geometries[0] = &GeometrySystem::acquireGeometry(configUI, true);
    ui1Mesh.generation = 0;

    //Setup events
    WorldRenderView& worldRenderView = *reinterpret_cast<WorldRenderView *>(masterRenderSystem.getRenderView("Fox_Fire_World_View"));
    worldRenderView.subToEvent(swapTextureEvent);
    EngineEvents::subscribe(KEY_PRESSED, [this](const EngineInputContext context) {quit();}, "Game.quit", KEY_ESCAPE);
    EngineEvents::subscribe(KEY_PRESSED, [this](const EngineInputContext context) {swapTextureEvent.call();}, "Game.swapTexture", KEY_1);
    EngineEvents::subscribe(KEY_PRESSED, [this](const EngineInputContext context) {loadModelsEvent.call();}, "Game.loadModels", KEY_2);
    loadModelsEvent.subscribe([this](){loadModels();});

    Engine::startup();
}

bool Game::update(const float deltaTime) {
    frameAllocator.freeAll();

    static unsigned long allocationCount = 0;
    const unsigned long previousAllocationCount = allocationCount;
    allocationCount = FF_Memory::getAllocationCount();
    if (inputSystem->isKeyUp(KEY_M) && inputSystem->wasKeyDown(KEY_M)) {
        const String usage = FF_Memory::getMemoryUsage();
        Logger::logDebug(usage);
        Logger::logDebug("Allocations: " + toString(allocationCount) + ". " + toString(allocationCount - previousAllocationCount) + " this frame.");
    }

    auto& state = *reinterpret_cast<GameState*>(gameInstance.state);
    Camera& camera = *EntityComponentSystem::getComponent<Camera>(state.worldCamera);

    if (inputSystem->isKeyDown(KEY_LEFT)) {
        CameraUtils::adjustYaw(camera, 1.0f * deltaTime);
    }
    if (inputSystem->isKeyDown(KEY_RIGHT)) {
        CameraUtils::adjustYaw(camera, -1.0f * deltaTime);
    }

    if (inputSystem->isKeyDown(KEY_UP)) {
        CameraUtils::adjustPitch(camera, 1.0f * deltaTime);
    }
    if (inputSystem->isKeyDown(KEY_DOWN)) {
        CameraUtils::adjustPitch(camera, -1.0f * deltaTime);
    }

    static constexpr float moveSpeed = 50.0f;

    if (inputSystem->isKeyDown(KEY_W)) {
        CameraUtils::moveForward(camera, moveSpeed * deltaTime);

    }
    if (inputSystem->isKeyDown(KEY_S)) {
        CameraUtils::moveBackward(camera, moveSpeed * deltaTime);
    }

    if (inputSystem->isKeyDown(KEY_A)) {
        CameraUtils::moveLeft(camera, moveSpeed * deltaTime);

    }
    if (inputSystem->isKeyDown(KEY_D)) {
        CameraUtils::moveRight(camera, moveSpeed * deltaTime);
    }

    if (inputSystem->isKeyDown(KEY_SPACE)) {
        CameraUtils::moveUp(camera, moveSpeed * deltaTime);
    }
    if (inputSystem->isKeyDown(KEY_LSHIFT)) {
        CameraUtils::moveDown(camera, moveSpeed * deltaTime);
    }

    if (inputSystem->isKeyUp(KEY_P) && inputSystem->wasKeyDown(KEY_P)) {
        Logger::logDebug("Camera Pos: " + toString(CameraUtils::getPosition(camera).x) + " " + toString(CameraUtils::getPosition(camera).y) + " " + toString(CameraUtils::getPosition(camera).z));
    }

    //Rotate the cubes
    const Quat rotation = getQuatFromAxisAngle({0, 1, 0}, 0.5f * deltaTime, false);
    TransformUtils::addRotation(*EntityComponentSystem::getComponent<Transform>(state.worldEntities[0]), rotation);
    TransformUtils::addRotation(*EntityComponentSystem::getComponent<Transform>(state.worldEntities[1]), rotation);
    TransformUtils::addRotation(*EntityComponentSystem::getComponent<Transform>(state.worldEntities[2]), rotation);

    //Update ui text
            const Camera& worldCamera = *EntityComponentSystem::getComponent<Camera>(CameraSystem::getDefaultCamera());
            const Vector3f position = CameraUtils::getPosition(worldCamera);
            const Vector3f uiRotation = CameraUtils::getRotation(worldCamera);

            //Get Mouse State
            const bool leftDown = inputSystem->isButtonDown(BUTTON_LEFT);
            const bool rightDown = inputSystem->isButtonDown(BUTTON_RIGHT);
            int mouseX = 0;
            int mouseY = 0;
            inputSystem->getMousePosition(mouseX, mouseY);

            //Convert to NDC
            const float ndcMouseX = convertFloatRange(static_cast<float>(mouseX), 0, state.width, -1, 1);
            const float ndcMouseY = convertFloatRange(static_cast<float>(mouseY), 0, state.height, -1, 1);

            //Calculate frame average
            double fps = 0;
            double frameTime = 0;
            engineMetrics.getFrameMetrics(fps, frameTime);

            //Display frame data on UI Text.
            String display = "-----Debug Data-----\nFPS: " + toString(fps, 1) + "(" + toString(frameTime, 1) + "ms)\n" +
                    "POS: [" + vector3fToString(position, 3) + "]\n" +
                        "ROT: [" + vector3fToString(uiRotation, 3) + "]\n" +
                            "Mouse: X=" + toString(mouseX, 0) + " Y=" + toString(mouseY, 0) + " L=" + toString(leftDown) + " R=" + toString(rightDown) + "\n" +
            "NDC: X=" + toString(ndcMouseX, 6) + " Y=" + toString(ndcMouseY, 6) + "\n" +
                "Hovered: ";
            display += state.hoveredId == INVALID_ID_U32 ? "None" : toString(state.hoveredId) + "\n";

            state.bitmapText.setText(display);

    return Engine::update(deltaTime);
}

void Game::initialize() {
    std::construct_at(reinterpret_cast<GameState *>(gameInstance.state));

    Engine::initialize();
}

bool Game::boot() {
    frameAllocator.initialize(MEBIBYTES(64));

    //Setup font config
    gameInstance.config.fontConfig.bAutoRelease = false;
    gameInstance.config.fontConfig.defaultBitmapFontCount = 1;
    gameInstance.config.fontConfig.defaultSystemFontCount = 1;
    gameInstance.config.fontConfig.maxBitmapFonts = 101;
    gameInstance.config.fontConfig.maxSystemFonts = 101;

    BitmapFontConfig bitmapFontConfig{};
    bitmapFontConfig.name = "Ubuntu-px24";
    bitmapFontConfig.resourceName = "Ubuntu-px24";
    bitmapFontConfig.size = 24;
    gameInstance.config.fontConfig.bitmapFontConfigs = FF_Memory::ff_allocate_recursive<BitmapFontConfig>(ARRAY, 1);
    gameInstance.config.fontConfig.bitmapFontConfigs[0] = bitmapFontConfig;

    SystemFontConfig systemFontConfig{};
    systemFontConfig.defaultSize = 24;
    systemFontConfig.name = "Ubuntu";
    systemFontConfig.resourceName = "Ubuntu";
    gameInstance.config.fontConfig.systemFontConfigs = FF_Memory::ff_allocate_recursive<SystemFontConfig>(ARRAY, 1);
    gameInstance.config.fontConfig.systemFontConfigs[0] = systemFontConfig;

    if (!configureRenderViews()) {
        Logger::logError("Failed to create render views during boot.");
        return false;
    }

    return true;
}

bool Game::render(float deltaTime, RenderPacket &renderPacket) {
    GameState& state = *reinterpret_cast<GameState *>(gameInstance.state);

    renderPacket.viewCount = 4;
    renderPacket.views = static_cast<RenderViewPacket *>(frameAllocator.allocate(sizeof(RenderViewPacket) * renderPacket.viewCount));
    for (unsigned int i = 0; i < renderPacket.viewCount; i++) {
        std::construct_at(&renderPacket.views[i]);
    }

    SkyboxPacketData skyboxPacketData{};
    skyboxPacketData.skybox = &state.skybox;
    if (!masterRenderSystem.buildPacket(masterRenderSystem.getRenderView("Fox_Fire_Skybox_View"), &skyboxPacketData, renderPacket.views[0], frameAllocator)) {
        Logger::logError("Failed to build skybox packet.");
        return false;
    }

    MeshPacketData worldPacketData{};
    unsigned int meshCount = 0;
    constexpr unsigned int maxMeshes = 10;
    const auto meshes = static_cast<unsigned int *>(frameAllocator.allocate(sizeof(unsigned int) * maxMeshes));
    for (const unsigned int worldEntity : state.worldEntities) {
        if (worldEntity != INVALID_ID_U32) {
            meshes[meshCount] = worldEntity;
            meshCount++;
        }
    }
    meshes[meshCount] = state.maxwell;
    meshCount++;
    worldPacketData.meshCount = meshCount;
    worldPacketData.meshes = meshes;
    if (!masterRenderSystem.buildPacket(masterRenderSystem.getRenderView("Fox_Fire_World_View"), &worldPacketData, renderPacket.views[1], frameAllocator)) {
        Logger::logError("Failed to build world packet.");
        return false;
    }

    UIPacketData uiPacketData{};
    unsigned int uiMeshCount = 0;
    constexpr unsigned int maxUIMeshes = 10;
    const auto uiMeshes = static_cast<unsigned int *>(frameAllocator.allocate(sizeof(unsigned int) * maxUIMeshes));
    for (const unsigned int uiEntity : state.uiEntities) {
        if (uiEntity != INVALID_ID_U32) {
            uiMeshes[uiMeshCount] = uiEntity;
            uiMeshCount++;
        }
    }
    uiPacketData.meshData.meshCount = uiMeshCount;
    uiPacketData.meshData.meshes = uiMeshes;
    uiPacketData.textCount = 2;
    const auto texts = static_cast<UIText **>(frameAllocator.allocate(sizeof(UIText*) * uiPacketData.textCount));
    texts[0] = &state.bitmapText;
    texts[1] = &state.systemText;
    uiPacketData.texts = texts;
    if (!masterRenderSystem.buildPacket(masterRenderSystem.getRenderView("Fox_Fire_UI_View"), &uiPacketData, renderPacket.views[2], frameAllocator)) {
        Logger::logError("Failed to build ui packet.");
        return false;
    }

    PickPacketData pickPacketData{};
    pickPacketData.uiMeshData = uiPacketData.meshData;
    pickPacketData.worldMeshData = worldPacketData;
    pickPacketData.texts = uiPacketData.texts;
    pickPacketData.textCount = uiPacketData.textCount;

    if (!masterRenderSystem.buildPacket(masterRenderSystem.getRenderView("Fox_Fire_Pick_View"), &pickPacketData, renderPacket.views[3], frameAllocator)) {
        Logger::logError("Failed to build pick packet.");
        return false;
    }

    return true;
}

void Game::resize(const unsigned short newWidth, const unsigned short newHeight) {
    GameState& state = *reinterpret_cast<GameState *>(gameInstance.state);

    state.width = newWidth;
    state.height = newHeight;

    state.bitmapText.setPosition({20, static_cast<float>(state.height - 150), 0});

    Engine::resize(newWidth, newHeight);
}
