//
// Created by cmorg on 11/1/2026.
//

#include "SystemFontLoader.h"

#include <filesystem>

#include "src/modules/engine/Library/StringUtils.h"

bool SystemFontLoader::importFoxSystemFont(FileHandler &file, const String &outName, String directoryPath, SystemFontResourceData &outResource) {
    String mainFile{};
    String directory{};

    String line{};
    ULong bytesRead{};

    while (file.readLine(line, 511, bytesRead)) {
        if (line.getAt(0) == '#') continue;

        const ULong index = line.find('=');

        if (index == INVALID_ID_U64) continue;

        String variable = line.substringLeft(index - 1);
        String value = line.substringRight(index + 1);
        variable.trim();
        value.trim();

        if (variable == "Main_File") {
            mainFile = value;
            continue;
        }
        if (variable == "Directory") {
            directory = value;
        }
    }

    file.closeFile();

    directoryPath = directoryPath + "/" + directory + "/";

    //Get all .ttf files in the directory
    unsigned int fileCount = 0;
    String* files = file.getAllFiles(directoryPath, ".ttf", fileCount);

    //Start by setting up the main file (index 0)
    outResource.fonts.initialize(fileCount);
    outResource.binarySize.initialize(fileCount);
    outResource.binaries.initialize(fileCount);

    SystemFont systemFont{"//" + mainFile}; //The added // is just to make the bottom function work.
    outResource.fonts.push({StringUtils::getFilenameNoExtensionFromPath(systemFont.name)});

    if (!file.openFile(directoryPath + mainFile, READ, true)) {
        Logger::logError("Failed to open file " + directoryPath + mainFile);
        return false;
    }

    ULong fileSize = 0;
    if (!file.getFileSize(fileSize)) {
        Logger::logError("Failed to obtain file size for FSF file.");
        return false;
    }

    outResource.binaries.push(FF_Memory::ff_allocate_raw(fileSize, memoryTag));
    outResource.binarySize.emplace();
    auto readMemory = static_cast<unsigned char *>(outResource.binaries[0]);
    if (!file.readAll(readMemory, outResource.binarySize[0])) {
        Logger::logError("Failed to read TTF file.");
        return false;
    }

    if (outResource.binarySize[0] != fileSize) {
        Logger::logWarn("Possible file corruption detected! Binary Size and TTF File Size do not match!");
    }

    file.closeFile();

    if (!outResource.binaries[0]) {
        Logger::logError("Failed to obtain TTF binary or TTF did not contain a Font.");
        return false;
    }

    //Now do the same for all variants
    for (unsigned int i = 0; i < fileCount; i++) {
        String ttf = files[i];

        String tempName = "//" + ttf;
        String name = StringUtils::getFilenameNoExtensionFromPath(tempName);
        if (name == outResource.fonts[0].name) continue;

        outResource.fonts.push({name});

        if (!file.openFile(directoryPath + ttf, READ, true)) {
            Logger::logError("Failed to open TTF variant file: " + directoryPath + ttf);
            file.closeFile();
            return false;
        }

        if (!file.getFileSize(fileSize)) {
            Logger::logError("Failed to obtain file size for variant TTF file.");
            return false;
        }

        outResource.binaries.push(FF_Memory::ff_allocate_raw(fileSize, memoryTag));
        outResource.binarySize.emplace();
        auto readMemory2 = static_cast<unsigned char *>(outResource.binaries[outResource.binaries.getLength() - 1]);
        if (!file.readAll(readMemory2, outResource.binarySize[outResource.binaries.getLength() - 1])) {
            Logger::logError("Failed to read variant TTF file.");
            return false;
        }

        if (outResource.binarySize[outResource.binaries.getLength() - 1] != fileSize) {
            Logger::logWarn("Possible file corruption detected! variant Binary Size and TTF File Size do not match!");
        }

        file.closeFile();

        if (!outResource.binaries[outResource.binaries.getLength() - 1]) {
            Logger::logError("Failed to obtain TTF binary or TTF did not contain a Font.");
            return false;
        }
    }

    FF_Memory::ff_free_recursive<String>(files, ARRAY, fileCount);

    return writeFSFB(outName, outResource);
}

bool SystemFontLoader::readFSFB(FileHandler &file, SystemFontResourceData &data) {
    ULong bytesRead = 0;
    unsigned int readSize = 0;

    BinaryHeader header{};
    readSize = sizeof(BinaryHeader);
    if (!file.read(readSize, &header, bytesRead)) {
        Logger::logError("Failed to obtain FSFB header data.");
        file.closeFile();
        return false;
    }

    if (header.id != FOXFIRE_BINARY_ID && header.type == RESOURCE_TYPE_SYSTEM_FONT) {
        Logger::logError("FSFB file header is not valid!");
        file.closeFile();
        return false;
    }

    //Also includes the main file.
    unsigned int variants = 0;
    readSize = sizeof(unsigned int);
    if (!file.read(readSize, &variants, bytesRead)) {
        Logger::logError("Failed to obtain Number of variants.");
        file.closeFile();
        return false;
    }

    data.fonts.initialize(variants);
    data.binaries.initialize(variants);
    data.binarySize.initialize(variants);

    for (unsigned int i = 0; i < variants; i++) {
        unsigned int stringCount = 0;

        readSize = sizeof(unsigned int);
        if (!file.read(readSize, &stringCount, bytesRead)) {
            Logger::logError("Failed to obtain char count for variant name.");
            file.closeFile();
            return false;
        }

        char* name = FF_Memory::ff_allocate_recursive<char>(ARRAY, stringCount);
        readSize = sizeof(char) * stringCount;
        if (!file.read(readSize, &name, bytesRead)) {
            Logger::logError("Failed to obtain variant name.");
            file.closeFile();
            return false;
        }
        data.fonts.push({name});
        FF_Memory::ff_free_recursive<char>(name, ARRAY, stringCount);

        readSize = sizeof(ULong);
        ULong binarySize = 0;
        if (!file.read(readSize, &binarySize, bytesRead)) {
            Logger::logError("Failed to obtain FSFB binary size.");
            file.closeFile();
            return false;
        }
        data.binarySize.push(binarySize);

        void* binary = FF_Memory::ff_allocate_raw(binarySize, memoryTag);
        readSize = binarySize;
        if (!file.read(readSize, &binary, bytesRead)) {
            Logger::logError("Failed to obtain FSFB binary data.");
            file.closeFile();
            return false;
        }
        data.binaries.push(binary);
        binary = nullptr;
    }

    return true;
}

bool SystemFontLoader::writeFSFB(String outFileName, SystemFontResourceData &resource) {
    return true;
}

SystemFontLoader::SystemFontLoader() {
    type = RESOURCE_TYPE_SYSTEM_FONT;
    path = "Textures/Fonts";
    memoryTag = RESOURCE;
    memorySize = sizeof(SystemFontLoader);
}

bool SystemFontLoader::load(const String name, Resource &outResource, const String basePath, ILoaderParameters *params) {
    if (name.isEmpty()) return false;

    FileHandler file{};
    constexpr int supportedTypeCount = 2;
    SupportedSystemFontFileType supportedFileTypes[supportedTypeCount]{};
    supportedFileTypes[0] = {".fsfb", SYSTEM_FONT_FILE_TYPE_FSFB, true};
    supportedFileTypes[1] = {".FoxSystemFont", SYSTEM_FONT_FILE_TYPE_FOX_SYSTEM_FONT, false};

    String finalPath = basePath + "/" + path + "/" + name;
    SystemFontFileType type = SYSTEM_FONT_FILE_TYPE_NOT_FOUND;

    for (unsigned int i = 0; i < supportedTypeCount; i++) {
        if (file.exists(finalPath + supportedFileTypes[i].extensions)) {
            if (file.openFile(finalPath + supportedFileTypes[i].extensions, READ, supportedFileTypes[i].bIsBinary)) {
                finalPath = finalPath + supportedFileTypes[i].extensions;
                type = supportedFileTypes[i].type;
                break;
            }
        }
    }

    if (type == SYSTEM_FONT_FILE_TYPE_NOT_FOUND) {
        Logger::logError("Failed to find system font for " + name);
        return false;
    }

    outResource.path = finalPath;
    SystemFontResourceData data{};

    bool result = false;
    switch (type) {
        case SYSTEM_FONT_FILE_TYPE_FOX_SYSTEM_FONT: {
            const String binaryName = basePath + "/" + path + "/" + name + ".fsfb";
            String directoryPath = basePath + "/" + path;
            result = importFoxSystemFont(file, binaryName, directoryPath, data);
            break;
        }
        case SYSTEM_FONT_FILE_TYPE_FSFB: {
            result = readFSFB(file, data);
            break;
        }
        case SYSTEM_FONT_FILE_TYPE_NOT_FOUND: {
            Logger::logError("Unable to open unsupported system font: " + name);
            result = false;
            break;
        }
    }

    file.closeFile();

    if (!result) {
        Logger::logError("Failed to process system font file " + finalPath);
        outResource.data = nullptr;
        outResource.dataSize = 0;
        return false;
    }

    outResource.data = FF_Memory::ff_allocate<SystemFontResourceData>(memoryTag);
    *static_cast<SystemFontResourceData *>(outResource.data) = data;
    outResource.dataSize = sizeof(SystemFontResourceData);

    for (unsigned int i = 0; i < data.fonts.getLength(); i++) {
        data.fonts[i].name.destroy();
    }
    data.fonts.shutdown();
    data.binaries.shutdown();
    data.binarySize.shutdown();

    return true;
}

void SystemFontLoader::unload(Resource &resource) {
    if (resource.data) {
        const auto data = static_cast<SystemFontResourceData *>(resource.data);
        for (unsigned int i = 0; i < data->fonts.getLength(); i++) {
            data->fonts[i].name.destroy();
            FF_Memory::ff_free_raw(data->binaries[i], data->binarySize[i], memoryTag);
        }

        data->fonts.shutdown();
        data->binaries.shutdown();
        data->binarySize.shutdown();
        FF_Memory::ff_free<SystemFontResourceData>(resource.data, memoryTag);
        resource.data = nullptr;
        resource.dataSize = 0;
        resource.loaderId = INVALID_ID_U32;
        resource.path.clear();
    }
}

