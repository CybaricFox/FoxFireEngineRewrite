//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) {
    if (beginFrame(packet.deltaTime)) {
        backend->updateGlobalState(projection, view, zeroVector3f(), oneVector4f(), 0);
        static float angle = 0.00f;
        //angle += angle / 10000;
        const Quat rotation = getQuatFromAxisAngle(forwardVector3(), angle, false);
        const Mat4 model = convertQuatToRotationMatrix(rotation, zeroVector3f());
        GeometryRenderData data{};
        data.id = 0;
        data.model = model;
        data.textures[0] = &defaultTexture;
        backend->updateEntity(data);

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

void MasterRenderSystem::createTexture(String name, const bool autoRelease, const int width, const int height, const int channelCount, const unsigned char *pixels, const bool isTransparent, Texture &outTexture) const {
    backend->createTexture(name, autoRelease, width, height, channelCount, pixels, isTransparent, outTexture);
}

void MasterRenderSystem::destroyTexture(Texture &texture) const {
    if (backend) backend->destroyTexture(texture);
}

bool MasterRenderSystem::beginFrame(const float deltaTime) const {
    return backend->beginFrame(deltaTime);
}

bool MasterRenderSystem::endFrame(const float deltaTime) const {
    const bool result = backend->endFrame(deltaTime);
    backend->incrementFrameNumber();
    return result;
}

void MasterRenderSystem::createDefaultTexture() {
    Logger::logDebug("Creating default texture");
    constexpr unsigned int dimensions = 256;
    constexpr unsigned int bpp = 4; //rgba
    constexpr unsigned pixelCount = dimensions * dimensions;
    unsigned char pixels[pixelCount * bpp];
    FF_Memory::ff_set(pixels, 255, sizeof(unsigned char) * pixelCount * bpp);

    for (unsigned long row = 0; row < dimensions; ++row) {
        for (unsigned long column = 0; column < dimensions; ++column) {
            const unsigned long index = (row * dimensions) + column;
            const unsigned long index_bpp = index * bpp;
            if (row % 2) {
                if (column % 2) {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            } else {
                if (!(column % 2)) {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            }
        }
    }

    createTexture("default", false, dimensions, dimensions, 4, pixels, false, defaultTexture);
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

    createDefaultTexture();

    projection = perspective(degreesToRadians(45.0f), 1280 / 720.0f, nearClip, farClip);
    view = createTranslationMatrix({0, 0, -30});
    view = invertMatrix(view);

    return true;
}

void MasterRenderSystem::shutdown() {
    destroyTexture(defaultTexture);
    delete backend;
    backend = nullptr;
}

MasterRenderSystem::~MasterRenderSystem() {
    shutdown();
}
