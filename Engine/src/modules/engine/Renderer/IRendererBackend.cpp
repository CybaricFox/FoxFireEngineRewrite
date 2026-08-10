//
// Created by cmorg on 7/2/2026.
//

#include "IRendererBackend.h"

#include "Vulkan/VulkanBackend.h"
#include "../Library/Logger.h"

IRendererBackend* IRendererBackend::create(const RendererBackendType type, PlatformState& newPlatformState, const GameInstance& gameInstance) {
    IRendererBackend* backend;

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

    backend->platformState = &newPlatformState;

    return backend;
}

IRendererBackend::~IRendererBackend() {

}
