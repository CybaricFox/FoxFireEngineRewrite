//
// Created by cmorg on 7/2/2026.
//

#include "IRendererBackend.h"

#include "Vulkan/VulkanBackend.h"
#include "../Library/Logger.h"

IRendererBackend* IRendererBackend::create(RendererBackendType type, IPlatformState &newPlatformState, const int majorV, const int minorV, const int patchV) {
    IRendererBackend* backend;

    if (type == VULKAN) {
        auto* vulkanBackend = new VulkanBackend();
        vulkanBackend->setVersion(majorV, minorV, patchV);
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

void IRendererBackend::setDefaultTextureReferences(Texture *diffuse, Texture *specular, Texture *normal) {
    defaultDiffuseTexture = diffuse;
    defaultSpecularTexture = specular;
    defaultNormalTexture = normal;
}

IRendererBackend::~IRendererBackend() {
    defaultDiffuseTexture = nullptr;
    defaultSpecularTexture = nullptr;
    defaultNormalTexture = nullptr;
}
