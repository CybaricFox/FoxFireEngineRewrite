//
// Created by cmorg on 8/3/2026.
//

#include "BinaryLoader.h"

BinaryLoader::BinaryLoader() {
    type = RESOURCE_TYPE_BINARY;
    memoryTag = ARRAY;
    memorySize = sizeof(BinaryLoader);
}

bool BinaryLoader::load(const String name, Resource &outResource, const String basePath, ILoaderParameters *params) {
    if (name.isEmpty()) return false;

    const String finalPath = basePath + path + "/" + name;

    FileHandler file{};
    if (!file.openFile(finalPath, READ, true)) {
        Logger::logError("Binary Loader failed to open file for reading: " + finalPath);
        return false;
    }

    outResource.path = finalPath;

    ULong fileSize = 0;
    if (!file.getFileSize(fileSize)) {
        Logger::logError("Binary Loader failed to get file size: " + finalPath);
        file.closeFile();
        return false;
    }

    auto resourceData = FF_Memory::ff_allocate_recursive<unsigned char>(ARRAY, fileSize);
    ULong readSize = 0;
    if (!file.readAll(resourceData, readSize)) {
        Logger::logError("Binary Loader failed to read file: " + finalPath);
        file.closeFile();
        return false;
    }

    file.closeFile();
    outResource.data = resourceData;
    outResource.dataSize = readSize;
    outResource.name = name;

    return true;
}
