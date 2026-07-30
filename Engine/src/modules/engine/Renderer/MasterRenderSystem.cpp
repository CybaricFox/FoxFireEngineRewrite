//
// Created by cmorg on 7/2/2026.
//

#include "MasterRenderSystem.h"

#include "../Library/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

bool MasterRenderSystem::drawFrame(const RenderPacket& packet) {
    if (beginFrame(packet.deltaTime)) {
        backend->updateGlobalState(projection, view, zeroVector3f(), oneVector4f(), 0);
        static float angle = 0.00f;
        //angle += (angle + 0.001) / 10000;
        const Quat rotation = getQuatFromAxisAngle(forwardVector3(), angle, false);
        const Mat4 model = convertQuatToRotationMatrix(rotation, zeroVector3f());
        GeometryRenderData data{};
        data.id = 0;
        data.model = model;
        data.textures[0] = &testTexture;
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

void MasterRenderSystem::onDebugEvent() {
    const String files[2] = {"obamnaSODA", "whoishe"};
    static char choice = 1;
    choice++;
    choice %= 2;

    loadTexture(testTexture, files[choice]);
}

bool MasterRenderSystem::beginFrame(const float deltaTime) const {
    return backend->beginFrame(deltaTime);
}

bool MasterRenderSystem::endFrame(const float deltaTime) const {
    const bool result = backend->endFrame(deltaTime);
    backend->incrementFrameNumber();
    return result;
}

//Generates a default texture during runtime so dependency on a file system isn't necessary for niche scenarios.
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
    defaultTexture.generation = INVALID_ID;
}

Texture MasterRenderSystem::createBlankTexture() {
    Texture texture{};
    texture.generation = INVALID_ID;
    return texture;
}

bool MasterRenderSystem::loadTexture(Texture& texture, const String &fileName, const String &subFolders) {
    String path = "Assets/" + subFolders + "/" + fileName + ".";
    return loadTextureHelper(texture, path, fileName);
}

bool MasterRenderSystem::loadTexture(Texture &texture, const String &fileName) {
    String path = "Assets/" + fileName + ".";
    return loadTextureHelper(texture, path, fileName);
}

bool MasterRenderSystem::loadTextureHelper(Texture &texture, String &path, const String &fileName) {
    constexpr int requiredChannelCount = 4;
    stbi_set_flip_vertically_on_load(true); //stb loads the image from down to top, this effectively makes it read top to down.
    //In the future, this should check for the extension automatically
    path.append("png");

    Texture tempTexture{};
    int width = static_cast<int>(tempTexture.width);
    int height = static_cast<int>(tempTexture.height);
    int channelCount = tempTexture.channelCount;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channelCount, requiredChannelCount);
    tempTexture.width = width;
    tempTexture.height = height;
    tempTexture.channelCount = channelCount;

    //overwrite the texture channel count with the one we are using
    tempTexture.channelCount = requiredChannelCount;

    if (!data) {
        if (stbi_failure_reason()) {
            Logger::logWarn("#1 Failed to load texture file: " + path + ": " + stbi_failure_reason());
        }
        return false;
    }

    const unsigned int currentGeneration = texture.generation;
    texture.generation = INVALID_ID;
    const unsigned long totalSize = tempTexture.width * tempTexture.height * requiredChannelCount;

    //transparency
    bool isTransparent = false;
    for (unsigned long i = 0; i < totalSize; i += requiredChannelCount) {
        const unsigned char a = data[i + 3];
        if (a < 255) {
            isTransparent = true;
            break;
        }
    }

    //This always fails... but then works anyway? commented out for now until I know why.
    //if (stbi_failure_reason()) {
    //    Logger::logWarn("#2 Failed to load texture file: " + path + ": " + stbi_failure_reason());
    //    return false;
    //}

    createTexture(fileName, false, static_cast<int>(tempTexture.width), static_cast<int>(tempTexture.height), tempTexture.channelCount, data, isTransparent, tempTexture);

    destroyTexture(texture);
    texture = tempTexture;

    if (currentGeneration == INVALID_ID) {
        texture.generation = 0;
    } else {
        texture.generation = currentGeneration + 1;
    }

    stbi_image_free(data);
    return true;
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

    backend->setDefaultTexture(defaultTexture);
    backend->clearFrameNumber();

    if (!backend->initialize(appName, platform, width, height)) {
        Logger::logFatal("Renderer Backend failed to initialize!");
        return false;
    }

    projection = perspective(degreesToRadians(45.0f), 1280 / 720.0f, nearClip, farClip);
    view = createTranslationMatrix({0, 0, -30});
    view = invertMatrix(view);

    createDefaultTexture();

    //load textures
    testTexture = createBlankTexture();

    return true;
}

void MasterRenderSystem::shutdown() {
    destroyTexture(defaultTexture);
    destroyTexture(testTexture);
    delete backend;
    backend = nullptr;
}

MasterRenderSystem::~MasterRenderSystem() {
    shutdown();
}
