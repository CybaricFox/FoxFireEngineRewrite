//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "TextureUtils.h"
#include "../Library/Logger.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"

Texture MasterRenderSystem::createBlankTexture() {
    Texture texture{};
    texture.generation = INVALID_ID_U32;
    return texture;
}

void MasterRenderSystem::regenerateRenderTargets() const {
    for (unsigned char i = 0; i < renderTargetCount; i++) {
        backend->destroyRenderTarget(worldRenderpass->getRenderTarget(i), false);
        backend->destroyRenderTarget(uiRenderpass->getRenderTarget(i), false);

        Texture* windowTexture = backend->getWindowAttachment(i);
        Texture* depthTexture = backend->getDepthAttachment();

        DynamicArray<Texture*> attachments{2};
        attachments.push(windowTexture);
        attachments.push(depthTexture);

        backend->createRenderTarget(2, attachments, *worldRenderpass, framebufferWidth, framebufferHeight, worldRenderpass->getRenderTarget(i));
        backend->createRenderTarget(1, attachments, *uiRenderpass, framebufferWidth, framebufferHeight, uiRenderpass->getRenderTarget(i));

        attachments.shutdown();
    }
}

void MasterRenderSystem::changeRenderMode(const Keys key) {
    switch (key) {
        case KEY_1: {
            Logger::logDebug("Render mode set to default.");
            renderMode = RENDER_VIEW_DEFAULT;
            break;
        }
        case KEY_2: {
            Logger::logDebug("Render mode set to lighting.");
            renderMode = RENDER_VIEW_LIGHTING;
            break;
        }
        case KEY_3: {
            Logger::logDebug("Render mode set to normals.");
            renderMode = RENDER_VIEW_NORMALS;
            break;
        }
        default: break;
    }
}

Material & MasterRenderSystem::acquireMaterial(const String &name) const {
    return materialSystem->acquireMaterial(name);
}

void MasterRenderSystem::releaseMaterial(const String &name) const {
    materialSystem->releaseMaterial(name);
}

bool MasterRenderSystem::initialize(const String &appName, Platform& platform, const GameInstance& gameInstance, ResourceSystem& resources) {
    framebufferWidth = 1280;
    framebufferHeight = 720;
    bIsCurrentlyResizing = false;
    framesSinceResizeRequested = 0;

    backend = IRendererBackend::create(VULKAN, platform.getPlatformState(), gameInstance);
    if (backend == nullptr) {
        Logger::logFatal("Failed to create the backend renderer!");
        return false;
    }

    backend->clearFrameNumber();

    RendererBackendConfig config{};
    config.appName = appName;
    config.func = [this]() {regenerateRenderTargets();};

    config.renderpassCount = 2;
    const String worldName = "Fox_Fire_World_Renderpass";
    const String uiName = "Fox_Fire_UI_Renderpass";
    RenderpassConfig configs[2]{};

    configs[0].name = worldName;
    configs[0].nextName = uiName;
    configs[0].renderArea = createVector4f(0, 0, 1280, 720);
    configs[0].clearColor = createVector4f(0.0f, 0.0f, 0.2f, 1.0f);
    configs[0].clearFlags = RENDERPASS_CLEAR_COLOR | RENDERPASS_CLEAR_DEPTH | RENDERPASS_CLEAR_STENCIL;

    configs[1].name = uiName;
    configs[1].prevName = worldName;
    configs[1].renderArea = createVector4f(0, 0, 1280, 720);
    configs[1].clearColor = createVector4f(0.0f, 0.0f, 0.2f, 1.0f);
    configs[1].clearFlags = RENDERPASS_CLEAR_NONE;

    config.configs = configs;

    if (!backend->initialize(platform, config, renderTargetCount, &resources)) {
        Logger::logFatal("Renderer Backend failed to initialize!");
        return false;
    }
    void* worldTargets = FF_Memory::ff_allocate(sizeof(RenderTarget) * renderTargetCount, ARRAY);
    void* uiTargets = FF_Memory::ff_allocate(sizeof(RenderTarget) * renderTargetCount, ARRAY);
    for (unsigned int i = 0; i < renderTargetCount; i++) {
        const auto worldTarget = reinterpret_cast<RenderTarget *>(static_cast<unsigned char *>(worldTargets) + (sizeof(RenderTarget) * i));
        std::construct_at(worldTarget);
        const auto uiTarget = reinterpret_cast<RenderTarget *>(static_cast<unsigned char *>(uiTargets) + (sizeof(RenderTarget) * i));
        std::construct_at(uiTarget);
    }

    worldRenderpass = backend->getRenderpass(worldName);
    worldRenderpass->setRenderTargetCount(renderTargetCount);
    worldRenderpass->setTargets(static_cast<RenderTarget *>(worldTargets));

    uiRenderpass = backend->getRenderpass(uiName);
    uiRenderpass->setRenderTargetCount(renderTargetCount);
    uiRenderpass->setTargets(static_cast<RenderTarget *>(uiTargets));

    regenerateRenderTargets();

    worldRenderpass->setRenderArea(createVector4f(0, 0, static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight)));
    uiRenderpass->setRenderArea(createVector4f(0, 0, static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight)));

    //UBOs
    worldProjection = perspective(degreesToRadians(45.0f), 1280 / 720.0f, nearClip, farClip);
    ambientColor = {0.25, 0.25, 0.25, 1};

    uiProjection = orthographic(0, 1280, 720, 0, -100, 100);
    uiView = invertMatrix(matrixIdentity());

    return true;
}

bool MasterRenderSystem::initializeTextureSystem(const unsigned int initialCapacity, ITextureSystem *system, ResourceSystem* resourceSystem) {
    textureSystem = system;
    TextureUtils::setTextureSystemRef(textureSystem);
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

bool MasterRenderSystem::initializeCameraSystem(const CameraSystemConfig &config, MasterEntityComponentSystem* ecsRef) {
    const bool result = cameraSystem.initialize(config, ecsRef);
    if (result) {
        currentCameraId = cameraSystem.getDefaultCamera();
    }

    return result;
}

void MasterRenderSystem::shutdown() {
    cameraSystem.shutdown();

    for (unsigned char i = 0; i < renderTargetCount; i++) {
        backend->destroyRenderTarget(worldRenderpass->getRenderTarget(i), true);
        backend->destroyRenderTarget(uiRenderpass->getRenderTarget(i), true);
    }
    std::destroy_at(&worldRenderpass->getRenderTarget(0));
    std::destroy_at(&uiRenderpass->getRenderTarget(0));
    FF_Memory::ff_free(&worldRenderpass->getRenderTarget(0), sizeof(RenderTarget) * renderTargetCount, ARRAY);
    FF_Memory::ff_free(&uiRenderpass->getRenderTarget(0), sizeof(RenderTarget) * renderTargetCount, ARRAY);

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

bool MasterRenderSystem::drawFrame(RenderPacket& packet) {
    backend->incrementFrameNumber();

    if (bIsCurrentlyResizing) {
        framesSinceResizeRequested++;

        if (framesSinceResizeRequested >= 30) {
            const auto width = static_cast<float>(framebufferWidth);
            const auto height = static_cast<float>(framebufferHeight);
            worldProjection = perspective(degreesToRadians(45.0f), width / height, nearClip, farClip);
            uiProjection = orthographic(0, width, height, 0, -100, 100);
            backend->resize(static_cast<unsigned short>(width), static_cast<unsigned short>(height));
            framesSinceResizeRequested = 0;
            bIsCurrentlyResizing = false;
        } else {
            return true;
        }
    }

    worldRenderpass->setRenderAreaZ(framebufferWidth);
    worldRenderpass->setRenderAreaW(framebufferHeight);
    uiRenderpass->setRenderAreaZ(framebufferWidth);
    uiRenderpass->setRenderAreaW(framebufferHeight);

    auto camera = MasterEntityComponentSystem::getComponent<Camera>(currentCameraId);
    if (camera == nullptr) {
        Logger::logError("Camera is invalid! Obtaining default camera");
        currentCameraId = cameraSystem.getDefaultCamera();
        camera = MasterEntityComponentSystem::getComponent<Camera>(currentCameraId);
        if (camera == nullptr) {
            Logger::logFatal("Default camera is invalid! Cannot draw frame!");
            return false;
        }
    }

    Mat4 view = CameraUtils::getViewMatrix(*camera);

    if (!backend->beginFrame(packet.deltaTime)) {
        return true;
    }

    const unsigned char attachmentIndex = backend->getWindowAttachmentIndex();

    if (!backend->beginRenderpass(*worldRenderpass, worldRenderpass->getRenderTarget(attachmentIndex))) {
        Logger::logError("Backend failed to begin world renderpass!");
        return false;
    }

    if (!shaderSystem.use(materialShaderId)) {
        Logger::logError("Failed to use material shader!");
        return false;
    }

    if (!materialSystem->applyGlobal(materialShaderId, &worldProjection, &view, &ambientColor, CameraUtils::getPosition(*camera), renderMode)) {
        Logger::logError("Failed to apply globals for materials!");
        return false;
    }

    unsigned int count = packet.geometryCount;
    for (unsigned int i = 0; i < count; i++) {
        Material* material = packet.geometries[i].geometry->material;
        if (!material) material = &materialSystem->getDefaultMaterial();

        bool needsUpdate = material->frameNumber != backend->getFrameNumber();
        if (!materialSystem->applyInstance(*material, needsUpdate)) {
            Logger::logWarn("Failed to apply material: " + material->name);
            continue;
        }

        material->frameNumber = backend->getFrameNumber();

        materialSystem->applyLocal(*material, &packet.geometries[i].model);
        backend->drawGeometry(packet.geometries[i], textureSystem->getDefaultDiffuseTexture(), materialSystem->getDefaultMaterial());
    }

    if (!backend->endRenderpass(*worldRenderpass)) {
        Logger::logFatal("Failed to end renderpass world!");
        return false;
    }

    if (!backend->beginRenderpass(*uiRenderpass, uiRenderpass->getRenderTarget(attachmentIndex))) {
        Logger::logError("Backend failed to begin ui renderpass!");
        return false;
    }

    if (!shaderSystem.use(uiShaderId)) {
        Logger::logError("Failed to use ui shader!");
        return false;
    }

    if (!materialSystem->applyGlobal(uiShaderId, &uiProjection, &view, nullptr, CameraUtils::getPosition(*camera), renderMode)) {
        Logger::logError("Failed to apply globals for uis!");
        return false;
    }

    count = packet.uiGeometryCount;
    for (unsigned int i = 0; i < count; i++) {
        Material* material = packet.uiGeometries[i].geometry->material;
        if (!material) material = &materialSystem->getDefaultMaterial();

        bool needsUpdate = material->frameNumber != backend->getFrameNumber();
        if (!materialSystem->applyInstance(*material, needsUpdate)) {
            Logger::logWarn("Failed to apply material: " + material->name);
            continue;
        }

        material->frameNumber = backend->getFrameNumber();

        materialSystem->applyLocal(*material, &packet.uiGeometries[i].model);
        backend->drawGeometry(packet.uiGeometries[i], textureSystem->getDefaultDiffuseTexture(), materialSystem->getDefaultMaterial());
    }

    if (!backend->endRenderpass(*uiRenderpass)) {
        Logger::logFatal("Failed to end renderpass ui!");
        return false;
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

Texture & MasterRenderSystem::acquireTexture(const bool autoRelease, const String &fileName, const TextureUseCase useCase) const {
    return textureSystem->acquireTexture(autoRelease, false, fileName, useCase);
}

void MasterRenderSystem::releaseTexture(const String &name) const {
    textureSystem->releaseTexture(name);
}

//Vertex and index arrays must be freed upon disposal!
Geometry & MasterRenderSystem::acquireGeometry(GeometryConfig &config, const bool autoRelease) const {
    return geometrySystem->acquireGeometry(config, autoRelease);
}

GeometryConfig MasterRenderSystem::generatePlaneConfig(const float width, const float height, const unsigned int xCount,
                                                       const unsigned int yCount, const float xTile, const float yTile,
                                                       const String &name, const String &materialName) const {

    return geometrySystem->generatePlaneConfig(width, height, xCount, yCount, xTile, yTile, name, materialName);
}

GeometryConfig MasterRenderSystem::generateCubeConfig(const float width, const float height, const float depth, const float xTile, const float yTile, const String &name, const String &materialName) const {
    return geometrySystem->generateCubeConfig(width, height, depth, xTile, yTile, name, materialName);
}
