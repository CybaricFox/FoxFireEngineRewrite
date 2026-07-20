//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) const {
    if (beginFrame(packet.deltaTime)) {
        backend->updateGlobalState(projection, view, zeroVector3f(), oneVector4f(), 0);
        static float angle = 0.01f;
        angle += 0.001f;
        const Quat rotation = getQuatFromAxisAngle(forwardVector3(), angle, false);
        const Mat4 model = convertQuatToRotationMatrix(rotation, zeroVector3f());
        backend->updateObject(model);

        if (!endFrame(packet.deltaTime)) {
            Logger::logFatal("Failed to end frame!");
            return false;
        }
    }

    return true;
}

void MasterRenderSystem::onResize(const unsigned short width, const unsigned short height) {
    if (backend) {
        projection = perspective(degreesToRadians(45.0f), static_cast<float>(width) / static_cast<float>(height), nearClip, farClip);
        backend->resize(width, height);
    } else {
        Logger::logWarn("Backend cannot resize because it does not exist.");
    }
}

bool MasterRenderSystem::beginFrame(const float deltaTime) const {
    return backend->beginFrame(deltaTime);
}

bool MasterRenderSystem::endFrame(const float deltaTime) const {
    const bool result = backend->endFrame(deltaTime);
    backend->incrementFrameNumber();
    return result;
}

void MasterRenderSystem::setView(const Mat4 &newView) {
    view = newView;
}

bool MasterRenderSystem::initialize(const String &appName, Platform& platform, const GameInstance& gameInstance, const unsigned int width, const unsigned int height) {
    backend = RendererBackend::create(VULKAN, platform.getPlatformState(), gameInstance);
    if (backend == nullptr) {
        Logger::logFatal("Failed to create the backend renderer!");
        return false;
    }
    backend->clearFrameNumber();

    if (!backend->initialize(appName, platform, width, height)) {
        Logger::logFatal("Renderer Backend failed to initialize!");
        return false;
    }

    projection = perspective(degreesToRadians(45.0f), 1280 / 720.0f, nearClip, farClip);
    view = createTranslationMatrix({0, 0, -30});
    view = invertMatrix(view);

    return true;
}

void MasterRenderSystem::shutdown() {
    delete backend;
    backend = nullptr;
}

MasterRenderSystem::~MasterRenderSystem() {
    shutdown();
}
