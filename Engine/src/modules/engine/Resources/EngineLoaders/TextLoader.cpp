//
// Created by cmorg on 8/3/2026.
//

#include "TextLoader.h"

TextLoader::TextLoader() {
    type = RESOURCE_TYPE_TEXT;
}

bool TextLoader::load(const String name, Resource &outResource, const String basePath) {
    if (name.empty()) return false;

    const String finalPath = basePath + path + "/" + name;

    outResource.path = finalPath;

    FileHandler file{};
    if (!file.openFile(finalPath, READ, false)) {
        Logger::logError("Text Loader failed to open file for reading: " + finalPath);
        return false;
    }

    unsigned long fileSize = 0;
    if (!file.getFileSize(fileSize)) {
        Logger::logError("Text Loader failed to get file size: " + finalPath);
        file.closeFile();
        return false;
    }

    String string{};
    unsigned long readSize = 0;
    const auto resourceData = static_cast<char *>(FF_Memory::ff_allocate(sizeof(char) * fileSize, ARRAY));
    if (!file.readAll(string, readSize)) {
        Logger::logError("Text Loader failed to read file: " + finalPath);
        file.closeFile();
        return false;
    }

    file.closeFile();

    string.copy(resourceData, readSize);

    outResource.data = resourceData;
    outResource.dataSize = readSize;
    outResource.name = name;

    return true;
}

void TextLoader::unload(Resource &resource) {
    if (resource.data) {
        FF_Memory::ff_free(resource.data, resource.dataSize, ARRAY);
        resource.data = nullptr;
        resource.dataSize = 0;
        resource.loaderId = INVALID_ID;
        resource.path.clear();
    }
}
