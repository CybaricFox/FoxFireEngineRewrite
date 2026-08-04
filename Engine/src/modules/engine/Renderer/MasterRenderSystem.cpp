//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) const {
    if (!backend->beginFrame(packet.deltaTime)) {
        return true;
    }

    if (!backend->beginRenderpass(ENGINE_RENDER_PASS_WORLD)) {
        Logger::logError("Backend failed to begin world renderpass!");
        return false;
    }

    backend->updateWorldGlobalState(worldProjection, worldView, zeroVector3f(), oneVector4f(), 0);

    unsigned int count = packet.geometryCount;
    for (unsigned int i = 0; i < count; i++) {
        backend->drawGeometry(packet.geometries[i], textureSystem->getDefaultTexture(), materialSystem->getDefaultMaterial());
    }

    if (!backend->endRenderpass(ENGINE_RENDER_PASS_WORLD)) {
        Logger::logFatal("Failed to end renderpass world!");
        return false;
    }

    if (!backend->beginRenderpass(ENGINE_RENDER_PASS_UI)) {
        Logger::logError("Backend failed to begin ui renderpass!");
        return false;
    }

    backend->updateUIGlobalState(uiProjection, uiView, 0);

    count = packet.uiGeometryCount;
    for (unsigned int i = 0; i < count; i++) {
        backend->drawGeometry(packet.uiGeometries[i], textureSystem->getDefaultTexture(), materialSystem->getDefaultMaterial());
    }

    if (!backend->endRenderpass(ENGINE_RENDER_PASS_UI)) {
        Logger::logFatal("Failed to end renderpass ui!");
        return false;
    }

    const bool result = backend->endFrame(packet.deltaTime);
    backend->incrementFrameNumber();
    if (!result) {
        Logger::logError("Failed to end frame!");
        return false;
    }

    return true;
}

void MasterRenderSystem::onResize(const unsigned short width, const unsigned short height) {
    if (backend) {
        worldProjection = perspective(degreesToRadians(45.0f), static_cast<float>(width) / static_cast<float>(height), nearClip, farClip);
        uiProjection = orthographic(0, width, height, 0, -100, 100);
        backend->resize(width, height);
    } else {
        Logger::logWarn("Backend cannot resize because it does not exist.");
    }
}

Texture & MasterRenderSystem::acquireTexture(const bool autoRelease, const String &fileName) const {
    return textureSystem->acquireTexture(autoRelease, fileName);
}

void MasterRenderSystem::releaseTexture(const String &name) const {
    textureSystem->releaseTexture(name);
}

//Vertex and index arrays must be freed upon disposal!
Geometry & MasterRenderSystem::acquireGeometry(const GeometryConfig &config, const bool autoRelease) const {
    return geometrySystem->acquireGeometry(config, autoRelease);
}

void MasterRenderSystem::createRenderpasses() {
    if (bIsInitialized || renderpassProfiles.getLength() == 0) return;

    for (const RenderpassProfile& profile : renderpassProfiles) {
        backend->createRenderpass(profile);
    }

    renderpassProfiles.shutdown();
}

void MasterRenderSystem::createRenderSystems() {
    if (bIsInitialized || renderSystemProfiles.getLength() == 0) return;

    for (const RenderSystemProfile& profile : renderSystemProfiles) {
        backend->createRenderSystem(profile);
    }

    renderSystemProfiles.shutdown();
}

void MasterRenderSystem::addRenderpassProfile(const RenderpassProfile &profile) {
    if (bIsInitialized) return;
    if (renderpassProfiles.getLength() == 0) renderpassProfiles.initialize(1);
    renderpassProfiles.push(profile);
}

void MasterRenderSystem::addRenderSystemprofile(const RenderSystemProfile &profile) {
    if (bIsInitialized) return;
    if (renderSystemProfiles.getLength() == 0) renderSystemProfiles.initialize(1);
    renderSystemProfiles.push(profile);
}

GeometryConfig MasterRenderSystem::generatePlaneConfig(const float width, const float height, const unsigned int xCount,
                                                       const unsigned int yCount, const float xTile, const float yTile,
                                                       const String &name, const String &materialName) const {

    return geometrySystem->generatePlaneConfig(width, height, xCount, yCount, xTile, yTile, name, materialName);
}

Texture MasterRenderSystem::createBlankTexture() {
    Texture texture{};
    texture.generation = INVALID_ID;
    return texture;
}

void MasterRenderSystem::setView(const Mat4 &newView) {
    worldView = newView;
}

bool MasterRenderSystem::initialize(const String &appName, Platform& platform, const GameInstance& gameInstance, const unsigned int width, const unsigned int height, ResourceSystem& resources) {
    backend = RendererBackend::create(VULKAN, platform.getPlatformState(), gameInstance);
    if (backend == nullptr) {
        Logger::logFatal("Failed to create the backend renderer!");
        return false;
    }

    backend->clearFrameNumber();

    createRenderpasses();
    createRenderSystems();
    bIsInitialized = true;
    if (!backend->initialize(appName, platform, width, height, resources)) {
        Logger::logFatal("Renderer Backend failed to initialize!");
        return false;
    }

    worldProjection = perspective(degreesToRadians(45.0f), 1280 / 720.0f, nearClip, farClip);
    worldView = createTranslationMatrix({0, 0, -30});
    worldView = invertMatrix(worldView);

    uiProjection = orthographic(0, 1280, 720, 0, -100, 100);
    uiView = invertMatrix(matrixIdentity());

    return true;
}

bool MasterRenderSystem::initializeTextureSystem(const unsigned int initialCapacity, ITextureSystem *system, ResourceSystem* resourceSystem) {
    textureSystem = system;
    return textureSystem->initialize(initialCapacity, backend, resourceSystem);
}

bool MasterRenderSystem::initializeMaterialSystem(const unsigned int initialCapacity, IMaterialSystem *system, ResourceSystem* resourceSystem) {
    materialSystem = system;
    return materialSystem->initialize(initialCapacity, textureSystem, backend, resourceSystem);
}

bool MasterRenderSystem::initializeGeometrySystem(const unsigned int initialCapacity, IGeometrySystem *system, ResourceSystem* resourceSystem) {
    geometrySystem = system;
    return geometrySystem->initialize(initialCapacity, backend, materialSystem, resourceSystem);
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
