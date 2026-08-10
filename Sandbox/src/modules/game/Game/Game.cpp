//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/Library/Logger.h"
#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"
#include "src/modules/system/FoxFire_Textures/FoxFire_GeometrySystem.h"
#include "src/modules/system/FoxFire_Textures/FoxFire_MaterialSystem.h"
#include "src/modules/system/FoxFire_Textures/FoxFire_TextureSystem.h"

void Game::recalculateView(GameState *state) {
    if (!state->bIsCameraDirty) return;

    const Mat4 rotation = createEuler(state->cameraEuler.x, state->cameraEuler.y, state->cameraEuler.z);
    const Mat4 translation = createTranslationMatrix(state->cameraPos);
    state->view = rotation * translation;
    state->view = invertMatrix(state->view);
    state->bIsCameraDirty = false;
}

void Game::increaseCameraYaw(GameState *state, const float amount) {
    state->cameraEuler.y += amount;
    state->bIsCameraDirty = true;
}

void Game::increaseCameraPitch(GameState *state, const float amount) {
    state->cameraEuler.x += amount;
    const float limit =  degreesToRadians(89.0f);
    state->cameraEuler.x = std::clamp(state->cameraEuler.x, -limit, limit);
    state->bIsCameraDirty = true;
}

void Game::increaseCameraRoll(GameState *state, const float amount) {
    state->cameraEuler.z += amount;
    state->bIsCameraDirty = true;
}

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
    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {quit();}, "Engine.quit", KEY_ESCAPE);

    //event system
    swapTextureEvent.subscribe([this]() {onDebugEvent();});
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

    if (inputSystem->isKeyDown(KEY_A) || inputSystem->isKeyDown(KEY_LEFT)) {
        increaseCameraYaw(state, 1.0f * deltaTime);
    }
    if (inputSystem->isKeyDown(KEY_D) || inputSystem->isKeyDown(KEY_RIGHT)) {
        increaseCameraYaw(state, -1.0f * deltaTime);
    }

    if (inputSystem->isKeyDown(KEY_UP)) {
        increaseCameraPitch(state, 1.0f * deltaTime);
    }
    if (inputSystem->isKeyDown(KEY_DOWN)) {
        increaseCameraPitch(state, -1.0f * deltaTime);
    }

    float moveSpeed = 50.0f;
    Vector3f velocity = zeroVector3f();

    if (inputSystem->isKeyDown(KEY_W)) {
        Vector3f forward = getForwardDirection(state->view);
        velocity += forward;

    }
    if (inputSystem->isKeyDown(KEY_S)) {
        Vector3f backward = getBackwardDirection(state->view);
        velocity += backward;
    }

    if (inputSystem->isKeyDown(KEY_Q)) {
        Vector3f left = getLeftDirection(state->view);
        velocity += left;

    }
    if (inputSystem->isKeyDown(KEY_E)) {
        Vector3f right = getRightDirection(state->view);
        velocity += right;
    }

    if (inputSystem->isKeyDown(KEY_SPACE)) {
        velocity.y += 1.0f;
    }
    if (inputSystem->isKeyDown(KEY_X)) {
        velocity.y -= 1.0f;
    }

    Vector3f z = zeroVector3f();
    if (!compareVectors(z, velocity, 0.0002f)) {
        normalize(&velocity);
        state->cameraPos += (velocity * moveSpeed * deltaTime);
        state->bIsCameraDirty = true;
    }

    //These should be removed eventually
    recalculateView(state);
    masterRenderSystem.setView(state->view);

    return Engine::update(deltaTime);
}

void Game::initialize() {
    const auto state = createGameState<GameState>();
    state->cameraPos = {0, 0, 30};
    state->cameraEuler = zeroVector3f();

    state->view = createTranslationMatrix(state->cameraPos);
    state->view = invertMatrix(state->view);
    state->bIsCameraDirty = true;

    //User defined renderpasses
    RenderpassProfile mainProfile{};
    mainProfile.name = "Fox_Fire_World_Renderpass";
    mainProfile.id = 0;
    mainProfile.clearFlags = RENDERPASS_CLEAR_COLOR | RENDERPASS_CLEAR_DEPTH | RENDERPASS_CLEAR_STENCIL;
    mainProfile.clearColor = {0, 0, 0.2, 1};
    masterRenderSystem.addRenderpassProfile(mainProfile);

    RenderpassProfile uiProfile{};
    uiProfile.name = "Fox_Fire_UI_Renderpass";
    uiProfile.id = 1;
    uiProfile.clearFlags = RENDERPASS_CLEAR_NONE;
    uiProfile.clearColor = {0, 0, 0, 0};
    masterRenderSystem.addRenderpassProfile(uiProfile);

    Engine::initialize();
}
