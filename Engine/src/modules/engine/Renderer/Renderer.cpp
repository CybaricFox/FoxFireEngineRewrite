//
// Created by cmorg on 7/2/2026.
//

#include "Renderer.h"

#include "../Library/Logger.h"

bool Renderer::drawFrame(const RenderPacket *packet, RendererBackend *backend) {
    if (beginFrame(packet->deltaTime, backend)) {
        if (!endFrame(packet->deltaTime, backend)) {
            Logger::logFatal("Failed to end frame!");
            return false;
        }
    }

    return true;
}

void Renderer::onResize(const unsigned short width, const unsigned short height, RendererBackend* backend) {
    if (backend) {
        backend->resize(width, height);
    } else {
        Logger::logWarn("Backend cannot resize because it does not exist.");
    }
}

bool Renderer::beginFrame(const float deltaTime, RendererBackend* backend) {
    return backend->beginFrame(deltaTime);
}

bool Renderer::endFrame(const float deltaTime, RendererBackend* backend) {
    const bool result = backend->endFrame(deltaTime);
    backend->incrementFrameNumber();
    return result;
}

bool Renderer::initialize(const String &appName, Platform* platform, RendererBackend*& backend, const GameInstance *gameInstance, const unsigned int width, const unsigned int height) {
    backend = RendererBackend::create(VULKAN, platform->getPlatformState(), gameInstance);
    if (backend == nullptr) {
        Logger::logFatal("Failed to create the backend renderer!");
        return false;
    }
    backend->clearFrameNumber();

    if (!backend->initialize(appName, platform, width, height)) {
        Logger::logFatal("Renderer Backend failed to initialize!");
        return false;
    }

    return true;
}

Renderer::~Renderer() {

}
