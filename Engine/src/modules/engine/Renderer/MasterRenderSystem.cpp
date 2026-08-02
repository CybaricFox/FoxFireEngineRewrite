//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) const {
    if (beginFrame(packet.deltaTime)) {
        backend->updateGlobalState(projection, view, zeroVector3f(), oneVector4f(), 0);

        const unsigned int count = packet.geometryCount;
        for (unsigned int i = 0; i < count; i++) {
            backend->drawGeometry(packet.geometries[i], textureSystem->getDefaultTexture(), materialSystem->getDefaultMaterial());
        }

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

Texture & MasterRenderSystem::acquireTexture(const bool autoRelease, const String &fileName, const String &subPath) const {
    return textureSystem->acquireTexture(autoRelease, fileName, subPath);
}

void MasterRenderSystem::releaseTexture(const String &name) const {
    textureSystem->releaseTexture(name);
}

//Vertex and index arrays must be freed upon disposal!
Geometry & MasterRenderSystem::acquireGeometry(const GeometryConfig &config, const bool autoRelease) const {
    return geometrySystem->acquireGeometry(config, autoRelease);
}

GeometryConfig MasterRenderSystem::generatePlaneConfig(const float width, const float height, const unsigned int xCount,
                                                       const unsigned int yCount, const float xTile, const float yTile,
                                                       const String &name, const String &materialName) const {

    return geometrySystem->generatePlaneConfig(width, height, xCount, yCount, xTile, yTile, name, materialName);
}

bool MasterRenderSystem::beginFrame(const float deltaTime) const {
    return backend->beginFrame(deltaTime);
}

bool MasterRenderSystem::endFrame(const float deltaTime) const {
    const bool result = backend->endFrame(deltaTime);
    backend->incrementFrameNumber();
    return result;
}

Texture MasterRenderSystem::createBlankTexture() {
    Texture texture{};
    texture.generation = INVALID_ID;
    return texture;
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

bool MasterRenderSystem::initializeTextureSystem(const unsigned int initialCapacity, ITextureSystem *system) {
    textureSystem = system;
    return textureSystem->initialize(initialCapacity, backend);
}

bool MasterRenderSystem::initializeMaterialSystem(const unsigned int initialCapacity, IMaterialSystem *system) {
    materialSystem = system;
    return materialSystem->initialize(initialCapacity, textureSystem, backend);
}

bool MasterRenderSystem::initializeGeometrySystem(const unsigned int initialCapacity, IGeometrySystem *system) {
    geometrySystem = system;
    return geometrySystem->initialize(initialCapacity, backend, materialSystem);
}

void MasterRenderSystem::shutdown() {
    if (geometrySystem) {
        delete geometrySystem;
        geometrySystem = nullptr;
    }
    if (materialSystem) {
        delete materialSystem;
        materialSystem = nullptr;
    }
    //Destroy texture system
    if (textureSystem) {
        delete textureSystem;
        textureSystem = nullptr;
    }

    delete backend;
    backend = nullptr;
}

MasterRenderSystem::~MasterRenderSystem() {
    shutdown();
}
