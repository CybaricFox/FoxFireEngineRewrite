//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) {
    if (beginFrame(packet.deltaTime)) {
        backend->updateGlobalState(projection, view, zeroVector3f(), oneVector4f(), 0);
        static float angle = 0.001f;
        //angle += angle / 10000;
        //const Quat rotation = getQuatFromAxisAngle(forwardVector3(), angle, false);
        //const Mat4 model = convertQuatToRotationMatrix(rotation, zeroVector3f());
        const Mat4 model = createTranslationMatrix({0, 0 , 0});
        GeometryRenderData data{};
        data.model = model;

        //Create a default material if none exists
        if (!testMaterial) {
            testMaterial = &materialSystem->acquireMaterial("MaterialTemplate", "templates");
            if (testMaterial == nullptr) {
                Logger::logWarn("Failed to acquire material for drawing, falling back to default!");
                MaterialConfig materialConfig{};
                materialConfig.name = "MaterialTemplate";
                materialConfig.bAutoRelease = false;
                materialConfig.diffuseColor = oneVector4f();
                materialConfig.mapName = DEFAULT_TEXTURE_NAME;
                testMaterial = &materialSystem->acquireMaterial(materialConfig);
            }
        }

        data.material = testMaterial;
        backend->updateEntity(data, textureSystem->getDefaultTexture());

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

void MasterRenderSystem::onDebugEvent() const {
    const String files[2] = {"obamnaSODA", "whoishe"};
    static char choice = 1;
    const String oldName = files[choice];
    choice++;
    choice %= 2;

    testMaterial->diffuseMap.texture = &textureSystem->acquireTexture(true, files[choice], "");
    if (!testMaterial->diffuseMap.texture) {
        Logger::logWarn("Master Render System debug event failed to obtain a texture!");
        testMaterial->diffuseMap.texture = &textureSystem->getDefaultTexture();
    }

    textureSystem->releaseTexture(oldName);
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

void MasterRenderSystem::shutdown() {
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
