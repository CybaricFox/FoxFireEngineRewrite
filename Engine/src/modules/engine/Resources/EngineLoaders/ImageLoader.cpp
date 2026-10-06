//
// Created by cmorg on 8/3/2026.
//

#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include <filesystem>

#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Renderer/stb/stb_image.h"

ImageLoader::ImageLoader() {
    type = RESOURCE_TYPE_IMAGE;
    path = "Textures";
    memoryTag = TEXTURE;
    memorySize = sizeof(ImageLoader);
}

bool ImageLoader::load(const String name, Resource &outResource, const String basePath, ILoaderParameters* params) {
    if (name.isEmpty()) return false;
    if (params == nullptr) return false;

    const auto imageParams = reinterpret_cast<ImageParameters *>(params);

    constexpr int requiredChannelCount = 4;
    stbi_set_flip_vertically_on_load_thread(imageParams->bFlipY); //stb loads the image from down to top, this effectively makes it read top to down.

    String finalPath{};
    constexpr int IMAGE_EXTENSION_COUNT = 5;
    const String extensions[IMAGE_EXTENSION_COUNT] = {".tga", ".png", ".jpg", ".bmp", ".JPG"};
    bool found = false;
    for (const auto & extension : extensions) {
        finalPath = basePath + "/" + path + "/" += name + extension;
        if (std::filesystem::exists(finalPath.getAsCharString())) {
            found = true;
            break;
        }
    }

    outResource.path = finalPath;
    outResource.name = name;

    if (!found) {
        Logger::logError("Failed to load image file " + finalPath + " with any supported extension.");
        return false;
    }

    FileHandler file{};
    if (!file.openFile(finalPath, READ, true)) {
        Logger::logError("Failed to read image file " + finalPath);
        file.closeFile();
        return false;
    }

    ULong fileSize = 0;
    if (!file.getFileSize(fileSize)) {
        Logger::logError("Failed to get size of image file " + finalPath);
        file.closeFile();
        return false;
    }

    int width = 0;
    int height = 0;
    int channelCount = 0;

    auto data = static_cast<unsigned char *>(FF_Memory::ff_allocate(fileSize, TEXTURE));
    if (!data) {
        Logger::logError("Image Resource loader failed to allocate file: " + finalPath);
        file.closeFile();
        return false;
    }

    ULong bytesRead = 0;
    const bool result = file.readAll(data, bytesRead);
    file.closeFile();

    if (!result) {
        Logger::logError("Image Resource Loader failed to read file: " + finalPath);
        return false;
    }

    if (bytesRead != fileSize) {
        Logger::logError("Image Resource Loader read " + toString(bytesRead) + " bytes but the file size is " + toString(fileSize));
        return false;
    }

    unsigned char* stbData = stbi_load_from_memory(data, static_cast<int>(fileSize), &width, &height, &channelCount, requiredChannelCount);
    if (!stbData) {
        Logger::logError("Image Resource Loader failed to load file: " + finalPath);
        return false;
    }

    const auto resourceData = static_cast<ImageResourceData *>(FF_Memory::ff_allocate(sizeof(ImageResourceData), TEXTURE, alignof(ImageResourceData)));
    resourceData->pixels = stbData;
    resourceData->width = width;
    resourceData->height = height;
    resourceData->channelCount = requiredChannelCount;
    outResource.data = resourceData;
    outResource.dataSize = sizeof(ImageResourceData);

    FF_Memory::ff_free(data, fileSize, TEXTURE);

    return true;
}

void ImageLoader::unload(Resource &resource) {
    const auto resourceData = static_cast<ImageResourceData *>(resource.data);
    if (resourceData) {
        stbi_image_free(resourceData->pixels);
    }

    ResourceLoader::unload(resource);
}
