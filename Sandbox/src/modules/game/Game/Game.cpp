//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"
#include "src/modules/engine/Library/Logger.h"
#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"
#include "src/modules/system/FoxFire_Renders/UIRenderView.h"
#include "src/modules/system/FoxFire_Renders/WorldRenderView.h"
#include "src/modules/system/FoxFire_Textures/FoxFire_GeometrySystem.h"
#include "src/modules/system/FoxFire_Textures/FoxFire_MaterialSystem.h"
#include "src/modules/system/FoxFire_Textures/FoxFire_TextureSystem.h"

Game::Game(const GameInstance& instance)
    :Engine(instance)
{
    inputSystem = instantiateDerivedSubSystem<FoxFire_InputSystem>();
    textureSystem = instantiateDerivedSubSystem<FoxFire_TextureSystem>();
    materialSystem = instantiateDerivedSubSystem<FoxFire_MaterialSystem>();
    geometrySystem = instantiateDerivedSubSystem<FoxFire_GeometrySystem>();
}

Game::~Game() {
    swapTextureEvent.destroyEvent();
}

void Game::startup() {
    //Assign the camera
    reinterpret_cast<GameState *>(gameInstance.state)->worldCamera = getDefaultCamera();

    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {quit();}, "Engine.quit", KEY_ESCAPE);

    //event system
    WorldRenderView& worldRenderView = *reinterpret_cast<WorldRenderView *>(getRenderView("Fox_Fire_World_View"));
    worldRenderView.subToEvent(swapTextureEvent);

    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {swapTextureEvent.call();}, "Game.swapTexture", KEY_L);

    Engine::startup();
}

bool Game::update(const float deltaTime) {
    static unsigned long allocationCount = 0;
    const unsigned long previousAllocationCount = allocationCount;
    allocationCount = FF_Memory::getAllocationCount();
    if (inputSystem->isKeyUp(KEY_M) && inputSystem->wasKeyDown(KEY_M)) {
        Logger::logDebug("Allocations: " + std::to_string(allocationCount) + ". " + std::to_string(allocationCount - previousAllocationCount) + " this frame.");
    }

    auto* state = reinterpret_cast<GameState*>(gameInstance.state);
    Camera& camera = *MasterEntityComponentSystem::getComponent<Camera>(state->worldCamera);

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
        Logger::logDebug("Camera Pos: " + std::to_string(CameraUtils::getPosition(camera).x) + " " + std::to_string(CameraUtils::getPosition(camera).y) + " " + std::to_string(CameraUtils::getPosition(camera).z));
    }

    return Engine::update(deltaTime);
}

void Game::initialize() {
    createGameState<GameState>();

    swapTextureEvent.registerEvent();

    Engine::initialize();
}
