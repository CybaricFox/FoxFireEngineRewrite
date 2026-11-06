//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/ECS/Engine_Components/UITextContainer.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/MeshUtils.h"
#include "src/modules/engine/Library/Logger.h"
#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"
#include "src/modules/system/FoxFire_Renders/SkyboxRenderView.h"
#include "src/modules/system/FoxFire_Renders/UIRenderView.h"
#include "src/modules/system/FoxFire_Renders/WorldRenderView.h"

void Game::loadModels() {
    if (!bModelsLoaded) {
        Logger::logDebug("Loading models");
        bModelsLoaded = true;

        if (!MeshUtils::loadMeshFromResource("Maxwell", 4)) {
            Logger::logError("Failed to load Maxwell.");
        }
    }
}

Game::Game(const GameInstance& instance)
    :Engine(instance)
{
    inputSystem = instantiateDerivedSubSystem<FoxFire_InputSystem>();
}

Game::~Game() {
    swapTextureEvent.destroyEvent();
}

void Game::startup() {
    //Assign the camera
    reinterpret_cast<GameState *>(gameInstance.state)->worldCamera = getDefaultCamera();

    //Create RenderViews
    //These had to be moved to Engine for the time being.
    WorldRenderView& worldRenderView = *reinterpret_cast<WorldRenderView *>(getRenderView("Fox_Fire_World_View"));
    worldRenderView.subToEvent(swapTextureEvent);
    DynamicArray<unsigned int>& uiEntities = EntityComponentSystem::getAllEntitiesOfType("Basic_UI");
    UITextContainer* container = EntityComponentSystem::getComponent<UITextContainer>(uiEntities[0]);
    container->texts[0]->setPosition({20, height - 75.0f, 0});
    container->texts[1]->setPosition({50, 250, 0});
    uiEntities.shutdown();
    FF_Memory::ff_free<DynamicArray<unsigned int>>(&uiEntities, DYNAMIC_ARRAY);

    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {quit();}, "Engine.quit", KEY_ESCAPE);
    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {swapTextureEvent.call();}, "Game.swapTexture", KEY_1);
    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {loadModelsEvent.call();}, "Game.loadModels", KEY_2);

    loadModelsEvent.subscribe([this](){loadModels();});

    Engine::startup();
}

bool Game::update(const float deltaTime) {
    static unsigned long allocationCount = 0;
    const unsigned long previousAllocationCount = allocationCount;
    allocationCount = FF_Memory::getAllocationCount();
    if (inputSystem->isKeyUp(KEY_M) && inputSystem->wasKeyDown(KEY_M)) {
        const String usage = FF_Memory::getMemoryUsage();
        Logger::logDebug(usage);
        Logger::logDebug("Allocations: " + toString(allocationCount) + ". " + toString(allocationCount - previousAllocationCount) + " this frame.");
    }

    auto* state = reinterpret_cast<GameState*>(gameInstance.state);
    Camera& camera = *EntityComponentSystem::getComponent<Camera>(state->worldCamera);

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

    return Engine::update(deltaTime);
}

void Game::initialize() {
    createGameState<GameState>();

    swapTextureEvent.registerEvent();
    loadModelsEvent.registerEvent();

    Engine::initialize();
}
