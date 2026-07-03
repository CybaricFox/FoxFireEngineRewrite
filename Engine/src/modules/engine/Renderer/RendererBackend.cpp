//
// Created by cmorg on 7/2/2026.
//

#include "RendererBackend.h"

#include "VulkanBackend.h"
#include "../Library/Logger.h"

void RendererBackend::resize(short width, short height) {
}

bool RendererBackend::initialize(String appName, Platform *platform) {
    return true;
}

bool RendererBackend::beginFrame(float deltaTime) {
    return true;
}

bool RendererBackend::endFrame(float deltaTime) {
    return true;
}

RendererBackend* RendererBackend::create(const RendererBackendType type, PlatformState *newPlatformState, const GameInstance *gameInstance) {
    RendererBackend* backend;

    if (type == VULKAN) {
        auto* vulkanBackend = new VulkanBackend();
        vulkanBackend->setVersion(gameInstance);
        backend = vulkanBackend;
    } else if (type == DIRECTX) {
        return nullptr;
    } else {
        Logger::logFatal("Render Backend is not supported!");
        return nullptr;
    }

    backend->platformState = newPlatformState;

    return backend;
}

RendererBackend::~RendererBackend() {

}
