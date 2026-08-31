//
// Created by cmorg on 8/3/2026.
//

#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <filesystem>

#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Renderer/stb/stb_image.h"

ImageLoader::ImageLoader() {
    type = RESOURCE_TYPE_IMAGE;
    path = "Textures";
    memoryTag = TEXTURE;
    memorySize = sizeof(ImageLoader);
}

bool ImageLoader::load(const String name, Resource &outResource, const String basePath) {
    if (name.empty()) return false;

    constexpr int requiredChannelCount = 4;
    stbi_set_flip_vertically_on_load(true); //stb loads the image from down to top, this effectively makes it read top to down.

    String finalPath{};
    constexpr int IMAGE_EXTENSION_COUNT = 5;
    const String extensions[IMAGE_EXTENSION_COUNT] = {".tga", ".png", ".jpg", ".bmp", ".JPG"};
    bool found = false;
    for (const auto & extension : extensions) {
        finalPath = basePath + "/" + path + "/" += name + extension;
        if (std::filesystem::exists(finalPath)) {
            found = true;
            break;
        }
    }

    if (!found) {
        Logger::logError("Failed to load image file " + finalPath + " with any supported extension.");
        return false;
    }

    int width = 0;
    int height = 0;
    int channelCount = 0;
    unsigned char* data = stbi_load(finalPath.c_str(), &width, &height, &channelCount, requiredChannelCount);

    /*
    if (const char* failReason = stbi_failure_reason(); failReason) {
        Logger::logError("Image Resource loader failed to load file: " + finalPath + " because " + failReason);
        stbi__err(nullptr, 0);

        if (data) {
            stbi_image_free(data);
        }

        return false;
    }
    */

    if (!data) {
        Logger::logError("Image Resource loader failed to load file: " + finalPath);
        return false;
    }

    outResource.path = finalPath;
    const auto resourceData = static_cast<ImageResourceData *>(FF_Memory::ff_allocate(sizeof(ImageResourceData), TEXTURE));
    resourceData->pixels = data;
    resourceData->width = width;
    resourceData->height = height;
    resourceData->channelCount = requiredChannelCount;

    outResource.data = resourceData;
    outResource.dataSize = sizeof(ImageResourceData);
    outResource.name = name;

    return true;
}
