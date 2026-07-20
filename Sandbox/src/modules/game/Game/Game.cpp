//
// Created by cmorg on 7/1/2026.
//

#include "Game.h"

#include "src/modules/engine/Library/Logger.h"
#include "src/modules/system/FoxFire_Input/FoxFire_InputSystem.h"

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

Game::Game()
    :Engine()
{
    inputSystem = new FoxFire_InputSystem();
}

void Game::startup() {
    inputSystem->subscribeToEngineEvent(KEY_PRESSED, [this](const EngineInputContext context) {quit();}, "Engine.quit", KEY_ESCAPE);

    Engine::startup();
}

bool Game::update(const float deltaTime) {
    static unsigned long allocationCount = 0;
    const unsigned long previousAllocationCount = allocationCount;
    allocationCount = FF_Memory::getAllocationCount();
    if (inputSystem->isKeyUp(KEY_M) && inputSystem->wasKeyDown(KEY_M)) {
        Logger::logDebug("Allocations: " + std::to_string(allocationCount) + ". " + std::to_string(allocationCount - previousAllocationCount) + " this frame.");
    }

    auto* state = static_cast<GameState*>(gameInstance->state);

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
    setView(state->view);

    return Engine::update(deltaTime);
}

void Game::initialize(GameInstance &instance) {
    const auto state = static_cast<GameState *>(instance.state);
    state->cameraPos = {0, 0, 30};
    state->cameraEuler = zeroVector3f();

    state->view = createTranslationMatrix(state->cameraPos);
    state->view = invertMatrix(state->view);
    state->bIsCameraDirty = true;

    Engine::initialize(instance);
}
