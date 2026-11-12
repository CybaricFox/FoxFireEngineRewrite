//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/MeshUtils.h"

MasterRenderSystem* MasterRenderSystem::instance = nullptr;

Texture MasterRenderSystem::createBlankTexture() {
    Texture texture{};
    texture.generation = INVALID_ID_U32;
    return texture;
}

bool MasterRenderSystem::createRenderBuffer(const RenderBufferType type, const ULong size, const bool useFreeList, RenderBuffer &outBuffer) {
    outBuffer.initialize(type, size, useFreeList);
    if (!instance->backend->createBuffer(outBuffer)) {
        Logger::logFatal("Failed to create render buffer");
        return false;
    }

    return true;
}

bool MasterRenderSystem::bindRenderBuffer(RenderBuffer & buffer, const ULong offset) {
    return instance->backend->bindBuffer(buffer, offset);
}

void MasterRenderSystem::destroyRenderBuffer(RenderBuffer &buffer) {
    buffer.shutdown();
    instance->backend->destroyBuffer(buffer);
}

bool MasterRenderSystem::releaseInstanceResources(const Shader &shader, const unsigned int instanceId) {
    return instance->backend->releaseInstanceResources(shader, instanceId);
}

bool MasterRenderSystem::drawRenderBuffer(RenderBuffer & buffer, const ULong offset, const unsigned int elementCount, const bool bindOnly) {
    return instance->backend->drawBuffer(buffer, offset, elementCount, bindOnly);
}

bool MasterRenderSystem::resizeRenderBuffer(RenderBuffer &buffer, const ULong newSize) {
    bool result = buffer.resizeBuffer(newSize);
    if (!result) return false;

    result = instance->backend->resizeBuffer(buffer, newSize);
    if (result) {
        buffer.setTotalSize(newSize);
    } else {
        Logger::logError("Failed to resize backend render buffer");
    }
    return result;
}

bool MasterRenderSystem::loadRenderBufferRange(RenderBuffer & buffer, const ULong offset, const ULong size, const void *data) {
    return instance->backend->loadBufferRange(buffer, offset, size, data);
}

void MasterRenderSystem::destroyTexture(Texture &texture) {
    instance->backend->destroyTexture(texture);
}

void MasterRenderSystem::createWritableTexture(Texture &texture) {
    return instance->backend->createWritableTexture(texture);
}

bool MasterRenderSystem::acquireTextureMapResources(TextureMap &textureMap) {
    return instance->backend->acquireTextureMapResources(textureMap);
}

void MasterRenderSystem::releaseTextureMapResources(TextureMap &textureMap) {
    instance->backend->releaseTextureMapResources(textureMap);
}

bool MasterRenderSystem::beginRenderpass(Renderpass &renderpass, RenderTarget &renderTarget) {
    return instance->backend->beginRenderpass(renderpass, renderTarget);
}

void MasterRenderSystem::drawGeometry(const GeometryRenderData &data) {
    instance->backend->drawGeometry(data);
}

bool MasterRenderSystem::endRenderpass(Renderpass &renderpass) {
    return instance->backend->endRenderpass(renderpass);
}

void MasterRenderSystem::readPixel(Texture &texture, const unsigned int x, const unsigned int y, unsigned char **outData) {
    instance->backend->readPixel(texture, x, y, outData);
}

bool MasterRenderSystem::bindShaderGlobals(Shader &shader) {
    return instance->backend->bindShaderGlobals(shader);
}

bool MasterRenderSystem::createRenderView(RenderViewConfig &config) {
    return renderViewSystem.createRenderView(config);
}

bool MasterRenderSystem::buildPacket(IRenderView *renderView, void* meshData, RenderViewPacket &packet, LinearAllocator& allocator) {
    return renderView->buildPacket(meshData, packet, allocator);
}

bool MasterRenderSystem::acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, TextureMap **maps) {
    return instance->backend->acquireInstanceResources(shader, outInstanceId, maps);
}

bool MasterRenderSystem::initialize(const String &appName, Platform& platform, const GameInstance& gameInstance, ResourceSystem& resources) {
    instance = this;

    framebufferWidth = 1280;
    framebufferHeight = 720;
    bIsCurrentlyResizing = false;
    framesSinceResizeRequested = 0;

    backend = IRendererBackend::create(VULKAN, platform.getPlatformState(), gameInstance.config.gameVersionMajor, gameInstance.config.gameVersionMinor, gameInstance.config.gameVersionPatch);
    if (backend == nullptr) {
        Logger::logFatal("Failed to create the backend renderer!");
        return false;
    }

    backend->clearFrameNumber();

    RendererBackendConfig config{};
    config.appName = appName;

    if (!backend->initialize(platform, config, renderTargetCount)) {
        Logger::logFatal("Renderer Backend failed to initialize!");
        return false;
    }

    return true;
}

bool MasterRenderSystem::initializeTextureSystem(const unsigned int initialCapacity, ResourceSystem* resourceSystem) {
    const bool result = textureSystem.initialize(initialCapacity, backend);
    backend->setDefaultTextureReferences(&TextureSystem::getDefaultDiffuseTexture(), &TextureSystem::getDefaultSpecularTexture(), &TextureSystem::getDefaultNormalTexture());
    return result;
}

bool MasterRenderSystem::initializeMaterialSystem(const MaterialSystemConfig config, ResourceSystem* resourceSystem) {
    return materialSystem.initialize(config, backend);
}

bool MasterRenderSystem::initializeGeometrySystem(const unsigned int initialCapacity, ResourceSystem* resourceSystem) {
    return geometrySystem.initialize(initialCapacity, backend);
}

bool MasterRenderSystem::initializeShaderSystem(const ShaderSystemConfig& config, ResourceSystem& resources) {
    return shaderSystem.initialize(config, backend);
}

bool MasterRenderSystem::initializeCameraSystem(const CameraSystemConfig &config, EntityComponentSystem* ecsRef) {
    const bool result = cameraSystem.initialize(config, ecsRef);
    if (result) {

    }

    return result;
}

bool MasterRenderSystem::initializeRenderViewSystem(const RenderViewSystemConfig &config) {
    return renderViewSystem.initialize(config, backend);
}

bool MasterRenderSystem::initializeFontSystem(const FontSystemConfig &config) {
    return fontSystem.initialize(config, backend);
}

void MasterRenderSystem::shutdown() {
    cameraSystem.shutdown();

    renderViewSystem.shutdown();

    materialSystem.shutdown();

    shaderSystem.shutdown();

    fontSystem.shutdown();

    geometrySystem.shutdown();

    textureSystem.shutdown();

    delete backend;
    backend = nullptr;

    instance = nullptr;
}

unsigned int MasterRenderSystem::getWindowAttachmentCount() const {
    return backend->getWindowAttachmentCount();
}

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) {
    backend->incrementFrameNumber();

    if (bIsCurrentlyResizing) {
        framesSinceResizeRequested++;

        if (framesSinceResizeRequested >= 30) {
            const auto width = static_cast<float>(framebufferWidth);
            const auto height = static_cast<float>(framebufferHeight);
            renderViewSystem.resize(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
            backend->resize(static_cast<unsigned short>(width), static_cast<unsigned short>(height));

            renderViewSystem.resize(static_cast<unsigned int>(width), static_cast<unsigned int>(height));

            framesSinceResizeRequested = 0;
            bIsCurrentlyResizing = false;
        } else {
            Platform::ff_sleep(16);
            return true;
        }
    }

    if (!backend->beginFrame(packet.deltaTime)) {
        return true;
    }

    const unsigned char attachmentIndex = backend->getWindowAttachmentIndex();

    for (unsigned int i = 0; i < packet.viewCount; i++) {
        if (!renderViewSystem.render(*packet.views[i].renderView, packet.views[i], backend->getFrameNumber(), attachmentIndex)) {
            Logger::logError("Failed to render view " + toString(i));
            return false;
        }
    }

    const bool result = backend->endFrame(packet.deltaTime);
    if (!result) {
        Logger::logError("Failed to end frame!");
        return false;
    }

    return true;
}

void MasterRenderSystem::onResize(const unsigned short width, const unsigned short height) {
    if (backend) {
        bIsCurrentlyResizing = true;
        framebufferWidth = width;
        framebufferHeight = height;
        framesSinceResizeRequested = 0;
    } else {
        Logger::logWarn("Backend cannot resize because it does not exist.");
    }
}
