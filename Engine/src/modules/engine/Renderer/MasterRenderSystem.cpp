//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"

Texture MasterRenderSystem::createBlankTexture() {
    Texture texture{};
    texture.generation = INVALID_ID_U32;
    return texture;
}

void MasterRenderSystem::createRenderpasses() {
    if (bIsInitialized || renderpassProfiles.getLength() == 0) return;

    for (const RenderpassProfile& profile : renderpassProfiles) {
        backend->createRenderpass(profile);
    }

    renderpassProfiles.shutdown();
}

bool MasterRenderSystem::getRenderpassId(const String &name, unsigned char &outId) {
    if (name == "Fox_Fire_World_Renderpass") {
        outId = 0;
        return true;
    }
    if (name == "Fox_Fire_UI_Renderpass") {
        outId = 1;
        return true;
    }

    Logger::logError("There is no renderpass named " + name);
    outId = INVALID_ID_U8;
    return false;
}

bool MasterRenderSystem::initialize(const String &appName, Platform& platform, const GameInstance& gameInstance, const unsigned int width, const unsigned int height, ResourceSystem& resources) {
    backend = IRendererBackend::create(VULKAN, platform.getPlatformState(), gameInstance);
    if (backend == nullptr) {
        Logger::logFatal("Failed to create the backend renderer!");
        return false;
    }

    backend->clearFrameNumber();

    createRenderpasses();

    bIsInitialized = true;

    if (!backend->initialize(appName, platform, width, height, &resources)) {
        Logger::logFatal("Renderer Backend failed to initialize!");
        return false;
    }

    //UBOs
    worldProjection = perspective(degreesToRadians(45.0f), 1280 / 720.0f, nearClip, farClip);
    worldView = createTranslationMatrix({0, 0, -30});
    worldView = invertMatrix(worldView);
    ambientColor = {0.25, 0.25, 0.25, 1};

    uiProjection = orthographic(0, 1280, 720, 0, -100, 100);
    uiView = invertMatrix(matrixIdentity());

    return true;
}

bool MasterRenderSystem::initializeTextureSystem(const unsigned int initialCapacity, ITextureSystem *system, ResourceSystem* resourceSystem) {
    textureSystem = system;
    return textureSystem->initialize(initialCapacity, backend, resourceSystem);
}

bool MasterRenderSystem::initializeMaterialSystem(const MaterialSystemConfig config, IMaterialSystem *system, ResourceSystem* resourceSystem) {
    materialSystem = system;
    return materialSystem->initialize(config, textureSystem, backend, resourceSystem, &shaderSystem);
}

bool MasterRenderSystem::initializeGeometrySystem(const unsigned int initialCapacity, IGeometrySystem *system, ResourceSystem* resourceSystem) {
    geometrySystem = system;
    return geometrySystem->initialize(initialCapacity, backend, materialSystem, resourceSystem);
}

bool MasterRenderSystem::initializeShaderSystem(const ShaderSystemConfig& config, ResourceSystem& resources) {
    if (!shaderSystem.initialize(config, backend, textureSystem)) return false;

    //Shaders
    Resource configResource{};
    ShaderConfig* shaderConfig = nullptr;

    if (!resources.load(DEFAULT_MATERIAL_SHADER_NAME, RESOURCE_TYPE_SHADER, configResource)) {
        Logger::logFatal("Failed to load material shader!");
        return false;
    }
    shaderConfig = static_cast<ShaderConfig *>(configResource.data);
    if (!shaderSystem.createShader(*shaderConfig)) {
        Logger::logFatal("Failed to create shader from config!");
        return false;
    }
    resources.unload(configResource);
    materialShaderId = shaderSystem.getId(DEFAULT_MATERIAL_SHADER_NAME);

    if (!resources.load(DEFAULT_UI_SHADER_NAME, RESOURCE_TYPE_SHADER, configResource)) {
        Logger::logFatal("Failed to load ui shader!");
        return false;
    }
    shaderConfig = static_cast<ShaderConfig *>(configResource.data);
    if (!shaderSystem.createShader(*shaderConfig)) {
        Logger::logFatal("Failed to create shader from config!");
        return false;
    }
    resources.unload(configResource);
    uiShaderId = shaderSystem.getId(DEFAULT_UI_SHADER_NAME);

    return true;
}

void MasterRenderSystem::shutdown() {
    if (geometrySystem) {
        FF_Memory::ff_free_class<IGeometrySystem>(geometrySystem, geometrySystem->getMemorySize(), GAME);
        geometrySystem = nullptr;
    }
    if (materialSystem) {
        FF_Memory::ff_free_class<IMaterialSystem>(materialSystem, materialSystem->getMemorySize(), GAME);
        materialSystem = nullptr;
    }

    shaderSystem.shutdown();

    //Destroy texture system
    if (textureSystem) {
        FF_Memory::ff_free_class<ITextureSystem>(textureSystem, textureSystem->getMemorySize(), GAME);
        textureSystem = nullptr;
    }

    delete backend;
    backend = nullptr;
}

void MasterRenderSystem::setView(const Mat4 &newView, const Vector3f newViewPosition) {
    worldView = newView;
    viewPosition = newViewPosition;
}

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) {
    if (!backend->beginFrame(packet.deltaTime)) {
        return true;
    }

    if (!backend->beginRenderpass(0)) {
        Logger::logError("Backend failed to begin world renderpass!");
        return false;
    }

    if (!shaderSystem.use(materialShaderId)) {
        Logger::logError("Failed to use material shader!");
        return false;
    }

    if (!materialSystem->applyGlobal(materialShaderId, &worldProjection, &worldView, &ambientColor, &viewPosition)) {
        Logger::logError("Failed to apply globals for materials!");
        return false;
    }

    unsigned int count = packet.geometryCount;
    for (unsigned int i = 0; i < count; i++) {
        Material* material = packet.geometries[i].geometry->material;
        if (!material) material = &materialSystem->getDefaultMaterial();

        if (!materialSystem->applyInstance(*material)) {
            Logger::logWarn("Failed to apply material: " + material->name);
            continue;
        }

        materialSystem->applyLocal(*material, &packet.geometries[i].model);
        backend->drawGeometry(packet.geometries[i], textureSystem->getDefaultDiffuseTexture(), materialSystem->getDefaultMaterial());
    }

    if (!backend->endRenderpass(0)) {
        Logger::logFatal("Failed to end renderpass world!");
        return false;
    }

    if (!backend->beginRenderpass(1)) {
        Logger::logError("Backend failed to begin ui renderpass!");
        return false;
    }

    if (!shaderSystem.use(uiShaderId)) {
        Logger::logError("Failed to use ui shader!");
        return false;
    }

    if (!materialSystem->applyGlobal(uiShaderId, &uiProjection, &uiView, nullptr, nullptr)) {
        Logger::logError("Failed to apply globals for uis!");
        return false;
    }

    count = packet.uiGeometryCount;
    for (unsigned int i = 0; i < count; i++) {
        Material* material = packet.uiGeometries[i].geometry->material;
        if (!material) material = &materialSystem->getDefaultMaterial();

        if (!materialSystem->applyInstance(*material)) {
            Logger::logWarn("Failed to apply material: " + material->name);
            continue;
        }

        materialSystem->applyLocal(*material, &packet.uiGeometries[i].model);
        backend->drawGeometry(packet.uiGeometries[i], textureSystem->getDefaultDiffuseTexture(), materialSystem->getDefaultMaterial());
    }

    if (!backend->endRenderpass(1)) {
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

void MasterRenderSystem::addRenderpassProfile(const RenderpassProfile &profile) {
    if (bIsInitialized) return;
    if (renderpassProfiles.getLength() == 0) renderpassProfiles.initialize(1);
    renderpassProfiles.push(profile);
}

GeometryConfig MasterRenderSystem::generatePlaneConfig(const float width, const float height, const unsigned int xCount,
                                                       const unsigned int yCount, const float xTile, const float yTile,
                                                       const String &name, const String &materialName) const {

    return geometrySystem->generatePlaneConfig(width, height, xCount, yCount, xTile, yTile, name, materialName);
}

GeometryConfig MasterRenderSystem::generateCubeConfig(const float width, const float height, const float depth, const float xTile, const float yTile, const String &name, const String &materialName) const {
    return geometrySystem->generateCubeConfig(width, height, depth, xTile, yTile, name, materialName);
}
