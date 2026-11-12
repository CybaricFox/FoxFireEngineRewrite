//
// Created by cmorg on 10/14/2026.
//

#include "BitmapFontLoader.h"

#include "src/modules/engine/Library/StringUtils.h"

constexpr int REQUIRED_GLYPH_ELEMENT_COUNT = 9;
constexpr int REQUIRED_KERNING_ELEMENT_COUNT = 3;

bool BitmapFontLoader::importFoxFont(const FileHandler &file, const String &name, BitmapFontResourceData &outData) {
    String line{};
    ULong bytesRead = 0;
    unsigned int lineNumber = 0;
    unsigned int glyphsRead = 0;
    unsigned char pagesRead = 0;
    unsigned int kerningsRead = 0;
    int stage = 0;
    //Stages mark how to read the file
    // 0 = header info
    // 1 = glyph info
    // 2 = kerning info

    while (true) {
       lineNumber++;
        if (!file.readLine(line, 511, bytesRead)) {
            break;
        }

        if (bytesRead < 1 || line.isEmpty() || line.getAt(0) == '#') continue;

        const unsigned int subCount = line.findAll('=');
        if (subCount == 0) continue;

        if (stage == 0 && subCount > 1) stage = 1;
        if (stage == 1 && line.getAt(0) == 'K') stage = 2;

        switch (stage) {
            case 0: {
                bool result = false;
                const ULong index = line.find('=');
                String variable = line.substringLeft(index - 1);
                String value = line.substringRight(index + 1);
                variable.trim();
                value.trim();

                if (variable == "Name") {
                    outData.data.name = value;
                    continue;
                }
                if (variable == "Size") {
                    result = StringUtils::stringToUInt(value, outData.data.size);
                    if (!result) {
                        Logger::logError("Cannot read Font File! Failed to read " + variable + ":" + value + " as a UInt.");
                        return false;
                    }
                    outData.data.height = static_cast<int>(outData.data.size);
                    continue;
                }
                if (variable == "Width") {
                    result = StringUtils::stringToInt(value, outData.data.atlasSizeX);
                    if (!result) {
                        Logger::logError("Cannot read Font File! Failed to read " + variable + ":" + value + " as a Int.");
                        return false;
                    }
                    continue;
                }
                if (variable == "Height") {
                    result = StringUtils::stringToInt(value, outData.data.atlasSizeY);
                    if (!result) {
                        Logger::logError("Cannot read Font File! Failed to read " + variable + ":" + value + " as a Int.");
                        return false;
                    }
                    continue;
                }
                if (variable == "Glyph_Count") {
                    result = StringUtils::stringToUInt(value, outData.data.glyphCount);
                    if (!result) {
                        Logger::logError("Cannot read Font File! Failed to read " + variable + ":" + value + " as a UInt.");
                        return false;
                    }
                    if (outData.data.glyphCount == 0) {
                        Logger::logError("Glyph_Count must be greater than 0.");
                        return false;
                    }
                    outData.data.glyphs = FF_Memory::ff_allocate_recursive<FontGlyph>(ARRAY, outData.data.glyphCount);
                    continue;
                }
                if (variable == "Kerning_Count") {
                    result = StringUtils::stringToUInt(value, outData.data.kerningCount);
                    if (!result) {
                        Logger::logError("Cannot read Font File! Failed to read " + variable + ":" + value + " as a UInt.");
                        return false;
                    }
                    if (outData.data.kerningCount != 0) {
                        outData.data.kernings = FF_Memory::ff_allocate_recursive<FontKerning>(ARRAY, outData.data.kerningCount);
                    }
                    continue;
                }
                if (variable == "Page_Count") {
                    result = StringUtils::stringToUInt(value, outData.pageCount);
                    if (!result) {
                        Logger::logError("Cannot read Font File! Failed to read " + variable + ":" + value + " as a UInt.");
                        return false;
                    }
                    if (outData.pageCount == 0) {
                        Logger::logError("Page count must be greater than zero!");
                        return false;
                    }
                    outData.pages = FF_Memory::ff_allocate_recursive<FontPage>(ARRAY, outData.pageCount);

                    for (unsigned int i = 0; i < outData.pageCount; i++) {
                        outData.pages[i].file = outData.data.name;
                    }

                    continue;
                }

                Logger::logError("Unknown Line! Line number: " + toString(lineNumber));
                break;
            }
            case 1: {
                if (glyphsRead == outData.data.glyphCount) {
                    Logger::logError("Expected glyph count is " + toString(outData.data.glyphCount) + " but " + outData.data.name + " is reading more glyphs than expected!");
                    return false;
                }

                bool result = false;
                const unsigned int substringCount = line.findAll(',') + 1;
                if (substringCount != REQUIRED_GLYPH_ELEMENT_COUNT) {
                    Logger::logError("Glyph entries require " + toString(REQUIRED_GLYPH_ELEMENT_COUNT) + " elements but only " + toString(substringCount) + " elements were found on line " + toString(lineNumber) + "!");
                    return false;
                }

                const auto strings = FF_Memory::ff_allocate_recursive<String>(ARRAY, substringCount);

                line.recursiveSplit(',', strings);

                for (unsigned int i = 0; i < substringCount; i++) {
                    String subLine = strings[i];
                    subLine.trim();

                    const char first = subLine.getAt(0);

                    const ULong index = subLine.find('=');
                    String value = subLine.substringRight(index + 1);
                    value.trim();

                    switch (first) {
                        case 'G': {
                            result = StringUtils::stringToInt(value, outData.data.glyphs[glyphsRead].codepoint);
                            if (!result) {
                                Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an Int");
                                return false;
                            }
                            break;
                        }
                        case 'P': {
                            result = StringUtils::stringToUChar(value, outData.data.glyphs[glyphsRead].page);
                            if (!result) {
                                Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an UChar");
                                return false;
                            }
                            break;
                        }
                        case 'X': {
                            if (subLine.getAt(1) == '_') {
                                result = StringUtils::stringToShort(value, outData.data.glyphs[glyphsRead].xOffset);
                                if (!result) {
                                    Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an Short");
                                    return false;
                                }
                                break;
                            }
                            result = StringUtils::stringToUShort(value, outData.data.glyphs[glyphsRead].x);
                            if (!result) {
                                Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an UShort");
                                return false;
                            }
                            break;
                        }
                        case 'Y': {
                            if (subLine.getAt(1) == '_') {
                                result = StringUtils::stringToShort(value, outData.data.glyphs[glyphsRead].yOffset);
                                if (!result) {
                                    Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an Short");
                                    return false;
                                }
                                break;
                            }
                            result = StringUtils::stringToUShort(value, outData.data.glyphs[glyphsRead].y);
                            if (!result) {
                                Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an UShort");
                                return false;
                            }
                            break;
                        }
                        case 'W': {
                            result = StringUtils::stringToUShort(value, outData.data.glyphs[glyphsRead].width);
                            if (!result) {
                                Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an UShort");
                                return false;
                            }
                            break;
                        }
                        case 'H': {
                            result = StringUtils::stringToUShort(value, outData.data.glyphs[glyphsRead].height);
                            if (!result) {
                                Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an UShort");
                                return false;
                            }
                            break;
                        }
                        case 'A': {
                            result = StringUtils::stringToFloat(value, outData.data.glyphs[glyphsRead].advance);
                            if (!result) {
                                Logger::logError("Failed to read glyph value! Cannot convert " + subLine + " to an Short");
                                return false;
                            }
                            break;
                        }
                        default: {
                            Logger::logError("Unknown glyph value entry: " + subLine);
                            break;
                        }
                    }
                }

                FF_Memory::ff_free_recursive<String>(strings, ARRAY, substringCount);
                glyphsRead++;
                break;
            }
            case 2: {
                if (kerningsRead == outData.data.kerningCount) {
                    Logger::logError("Expected kerning count is " + toString(outData.data.kerningCount) + " but " + outData.data.name + " is reading more kernings than expected!");
                    return false;
                }

                bool result = false;
                const unsigned int substringCount = line.findAll(',') + 1;
                if (substringCount != REQUIRED_KERNING_ELEMENT_COUNT) {
                    Logger::logError("Kerning entries require " + toString(REQUIRED_KERNING_ELEMENT_COUNT) + " elements but only " + toString(substringCount) + " elements were found on line " + toString(lineNumber) + "!");
                    return false;
                }

                const auto strings = FF_Memory::ff_allocate_recursive<String>(ARRAY, substringCount);

                line.recursiveSplit(',', strings);

                for (unsigned int i = 0; i < substringCount; i++) {
                    String subLine = strings[i];
                    subLine.trim();

                    const char first = subLine.getAt(0);

                    const ULong index = subLine.find('=');
                    String value = subLine.substringRight(index + 1);
                    value.trim();

                    switch (first) {
                        case 'K': {
                            result = StringUtils::stringToInt(value, outData.data.kernings[kerningsRead].codepointA);
                            if (!result) {
                                Logger::logError("Failed to read kerning value! Cannot convert " + subLine + " to an Int");
                                return false;
                            }
                            break;
                        }
                        case 'N': {
                            result = StringUtils::stringToInt(value, outData.data.kernings[kerningsRead].codepointB);
                            if (!result) {
                                Logger::logError("Failed to read kerning value! Cannot convert " + subLine + " to an Int");
                                return false;
                            }
                            break;
                        }
                        case 'A': {
                            result = StringUtils::stringToFloat(value, outData.data.kernings[kerningsRead].amount);
                            if (!result) {
                                Logger::logError("Failed to read kerning value! Cannot convert " + subLine + " to an Int");
                                return false;
                            }
                            break;
                        }
                        default: {
                            Logger::logError("Unknown kerning value entry: " + subLine);
                            break;
                        }
                    }
                }

                FF_Memory::ff_free_recursive<String>(strings, ARRAY, substringCount);
                kerningsRead++;
                break;
            }
        }
    }

    return writeFoxfireBinaryFont(name, outData);
}

bool BitmapFontLoader::readFoxfireBinaryFont(FileHandler &file, BitmapFontResourceData &data) {
    ULong bytesRead = 0;
    unsigned int readSize = 0;

    //Get the header first
    BinaryHeader header{};
    readSize = sizeof(BinaryHeader);

    if (!file.read(readSize, &header, bytesRead)) {
        Logger::logError("Failed to read binary header!");
        file.closeFile();
        return false;
    }

    //Verify header
    if (header.id != FOXFIRE_BINARY_ID) {
        Logger::logError("Binary file is incompatible!");
        file.closeFile();
        return false;
    }
    if (header.type != RESOURCE_TYPE_BITMAP_FONT) {
        Logger::logError("Binary is not a Bitmap Font!");
        file.closeFile();
        return false;
    }

    //Get Data
    unsigned int nameSize = 0;
    readSize = sizeof(unsigned int);
    if (!file.read(readSize, &nameSize, bytesRead)) {
        Logger::logError("Failed to read name size!");
        file.closeFile();
        return false;
    }

    char* name = FF_Memory::ff_allocate_recursive<char>(ARRAY, nameSize);
    readSize = sizeof(char) * nameSize;
    if (!file.read(readSize, name, bytesRead)) {
        Logger::logError("Failed to read name!");
        file.closeFile();
        return false;
    }
    data.data.name.setString(name, nameSize);
    FF_Memory::ff_free_recursive<char>(name, ARRAY, nameSize);
    name = nullptr;


    readSize = sizeof(unsigned int);
    if (!file.read(readSize, &data.data.size, bytesRead)) {
        Logger::logError("Failed to read font size!");
        file.closeFile();
        return false;
    }

    readSize = sizeof(int);
    if (!file.read(readSize, &data.data.height, bytesRead)) {
        Logger::logError("Failed to read font height!");
        file.closeFile();
        return false;
    }

    readSize = sizeof(int);
    if (!file.read(readSize, &data.data.base, bytesRead)) {
        Logger::logError("Failed to read base!");
        file.closeFile();
        return false;
    }

    readSize = sizeof(int);
    if (!file.read(readSize, &data.data.atlasSizeX, bytesRead)) {
        Logger::logError("Failed to read atlas size x!");
        file.closeFile();
        return false;
    }

    readSize = sizeof(int);
    if (!file.read(readSize, &data.data.atlasSizeY, bytesRead)) {
        Logger::logError("Failed to read atlas size y!");
        file.closeFile();
        return false;
    }

    //Write pages
    readSize = sizeof(unsigned int);
    if (!file.read(readSize, &data.pageCount, bytesRead)) {
        Logger::logError("Failed to read page count!");
        file.closeFile();
        return false;
    }
    data.pages = FF_Memory::ff_allocate_recursive<FontPage>(ARRAY, data.pageCount);

    for (unsigned int i = 0; i < data.pageCount; i++) {
        readSize = sizeof(char);
        if (!file.read(readSize, &data.pages[i].id, bytesRead)) {
            Logger::logError("Failed to read page id!");
            file.closeFile();
            return false;
        }

        readSize = sizeof(unsigned int);
        if (!file.read(readSize, &nameSize, bytesRead)) {
            Logger::logError("Failed to read page name length!");
            file.closeFile();
            return false;
        }

        if (nameSize != 0) {
            name = FF_Memory::ff_allocate_recursive<char>(ARRAY, nameSize);
            readSize = sizeof(char) * nameSize;
            if (!file.read(readSize, name, bytesRead)) {
                Logger::logError("Failed to read page name!");
                file.closeFile();
                return false;
            }
            data.pages[i].file.setString(name, nameSize);
            FF_Memory::ff_free_recursive<char>(name, ARRAY, nameSize);
            name = nullptr;
        }
    }

    //Write Glyphs
    readSize = sizeof(unsigned int);
    if (!file.read(readSize, &data.data.glyphCount, bytesRead)) {
        Logger::logError("Failed to read glpyh count!");
        file.closeFile();
        return false;
    }
    data.data.glyphs = FF_Memory::ff_allocate_recursive<FontGlyph>(ARRAY, data.data.glyphCount);

    readSize = sizeof(FontGlyph) * data.data.glyphCount;
    if (!file.read(readSize, data.data.glyphs, bytesRead)) {
        Logger::logError("Failed to read glpyhs!");
        file.closeFile();
        return false;
    }

    //Write Kernings
    readSize = sizeof(unsigned int);
    if (!file.read(readSize, &data.data.kerningCount, bytesRead)) {
        Logger::logError("Failed to read kerning count!");
        file.closeFile();
        return false;
    }

    if (data.data.kerningCount > 0) {
        data.data.kernings = FF_Memory::ff_allocate_recursive<FontKerning>(ARRAY, data.data.kerningCount);
        readSize = sizeof(FontKerning) * data.data.kerningCount;
        if (!file.read(readSize, data.data.kernings, bytesRead)) {
            Logger::logError("Failed to read kernings!");
            file.closeFile();
            return false;
        }
    }

    file.closeFile();
    return true;
}

bool BitmapFontLoader::writeFoxfireBinaryFont(const String &filePath, const BitmapFontResourceData &data) {
    FileHandler file{};

    if (!file.openFile(filePath, WRITE, true)) {
        Logger::logError("Failed to open file " + filePath);
        return false;
    }

    ULong bytesWritten = 0;
    unsigned int writeSize = 0;

    //Write header
    BinaryHeader header{};
    header.id = FOXFIRE_BINARY_ID;
    header.type = RESOURCE_TYPE_BITMAP_FONT;
    header.version = 1;
    writeSize = sizeof(BinaryHeader);
    if (!file.write(writeSize, &header, bytesWritten)) {
        Logger::logError("Failed to write binary header!");
        file.closeFile();
        return false;
    }

    //Write data
    unsigned int nameLength = data.data.name.getSizeInBytes();
    writeSize = sizeof(unsigned int);
    if (!file.write(writeSize, &nameLength, bytesWritten)) {
        Logger::logError("Failed to write name length!");
        file.closeFile();
        return false;
    }

    writeSize = sizeof(char) * nameLength;
    if (!file.write(writeSize, data.data.name.getAsCharString(), bytesWritten)) {
        Logger::logError("Failed to write name!");
        file.closeFile();
        return false;
    }

    writeSize = sizeof(unsigned int);
    if (!file.write(writeSize, &data.data.size, bytesWritten)) {
        Logger::logError("Failed to write font size!");
        file.closeFile();
        return false;
    }

    writeSize = sizeof(int);
    if (!file.write(writeSize, &data.data.size, bytesWritten)) {
        Logger::logError("Failed to write font height!");
        file.closeFile();
        return false;
    }

    writeSize = sizeof(int);
    if (!file.write(writeSize, &data.data.base, bytesWritten)) {
        Logger::logError("Failed to write base!");
        file.closeFile();
        return false;
    }

    writeSize = sizeof(int);
    if (!file.write(writeSize, &data.data.atlasSizeX, bytesWritten)) {
        Logger::logError("Failed to write atlas size x!");
        file.closeFile();
        return false;
    }

    writeSize = sizeof(int);
    if (!file.write(writeSize, &data.data.atlasSizeY, bytesWritten)) {
        Logger::logError("Failed to write atlas size y!");
        file.closeFile();
        return false;
    }

    //Write pages
    writeSize = sizeof(unsigned int);
    if (!file.write(writeSize, &data.pageCount, bytesWritten)) {
        Logger::logError("Failed to write page count!");
        file.closeFile();
        return false;
    }

    for (unsigned int i = 0; i < data.pageCount; i++) {
        writeSize = sizeof(char);
        if (!file.write(writeSize, &data.pages[i].id, bytesWritten)) {
            Logger::logError("Failed to write page id!");
            file.closeFile();
            return false;
        }

        nameLength = data.data.name.getSizeInBytes();
        writeSize = sizeof(unsigned int);
        if (!file.write(writeSize, &nameLength, bytesWritten)) {
            Logger::logError("Failed to write page name length!");
            file.closeFile();
            return false;
        }

        String pageName = data.data.name;
        writeSize = sizeof(char) * nameLength;
        if (!file.write(writeSize, pageName.getAsCharString(), bytesWritten)) {
            Logger::logError("Failed to write page name!");
            file.closeFile();
            return false;
        }
    }

    //Write Glyphs
    writeSize = sizeof(unsigned int);
    if (!file.write(writeSize, &data.data.glyphCount, bytesWritten)) {
        Logger::logError("Failed to write glpyh count!");
        file.closeFile();
        return false;
    }

    writeSize = sizeof(FontGlyph) * data.data.glyphCount;
    if (!file.write(writeSize, data.data.glyphs, bytesWritten)) {
        Logger::logError("Failed to write glpyhs!");
        file.closeFile();
        return false;
    }

    //Write Kernings
    writeSize = sizeof(unsigned int);
    if (!file.write(writeSize, &data.data.kerningCount, bytesWritten)) {
        Logger::logError("Failed to write kerning count!");
        file.closeFile();
        return false;
    }

    if (data.data.kerningCount > 0) {
        writeSize = sizeof(FontKerning) * data.data.kerningCount;
        if (!file.write(writeSize, data.data.kernings, bytesWritten)) {
            Logger::logError("Failed to write kernings!");
            file.closeFile();
            return false;
        }
    }

    file.closeFile();
    return true;
}

BitmapFontLoader::BitmapFontLoader() {
    type = RESOURCE_TYPE_BITMAP_FONT;
    path = "Textures/Fonts";
    memoryTag = RESOURCE;
    memorySize = sizeof(BitmapFontLoader);
}

bool BitmapFontLoader::load(const String name, Resource &outResource, const String basePath, ILoaderParameters *params) {
    if (name.isEmpty()) return false;

    String finalPath = basePath + "/" + path + "/" + name ;
    FileHandler file{};

    constexpr unsigned int supportedTypeCount = 2;

    SupportedBitmapFontFileType supportedFileTypes[supportedTypeCount]{};

    //REMINDER: the lower the index, the higher the priority
    supportedFileTypes[0] = {".ffb", BITMAP_FONT_FILE_FOXFIRE_BINARY, true};
    supportedFileTypes[1] = {".FoxFont", BITMAP_FONT_FILE_FOXFONT, false};

    BitmapFontFileType type = BITMAP_FONT_FILE_NOT_FOUND;
    for (auto & supportedFileType : supportedFileTypes) {
        if (file.exists(finalPath + supportedFileType.extension)) {
            if (file.openFile(finalPath + supportedFileType.extension, READ, supportedFileType.bIsBinary)) {
                finalPath = finalPath + supportedFileType.extension;
                type = supportedFileType.type;
                break;
            }
        }
    }

    if (type == BITMAP_FONT_FILE_NOT_FOUND) {
        Logger::logError("Failed to find bitmap font file for " + finalPath);
        return false;
    }

    outResource.path = finalPath;

    BitmapFontResourceData resourceData{};
    resourceData.data.type = FONT_TYPE_BITMAP;

    bool result = false;
    switch (type) {
        case BITMAP_FONT_FILE_FOXFIRE_BINARY: {
            result = readFoxfireBinaryFont(file, resourceData);
            break;
        }
        case BITMAP_FONT_FILE_FOXFONT: {
            const String binaryName = basePath + "/" + path + "/" + name + ".ffb";
            result = importFoxFont(file, binaryName, resourceData);
            break;
        }
        default: {
            Logger::logError("Reached switch but failed to find bitmap font file for " + finalPath);
            result = false;
            break;
        }
    }

    file.closeFile();

    if (!result) {
        Logger::logError("Failed to process bitmap font file for " + finalPath);
        outResource.path.destroy();
        outResource.data = nullptr;
        outResource.dataSize = 0;
        return false;
    }

    outResource.data = FF_Memory::ff_allocate<BitmapFontResourceData>(memoryTag);
    *static_cast<BitmapFontResourceData *>(outResource.data) = resourceData;
    outResource.dataSize = sizeof(BitmapFontResourceData);

    return true;
}

void BitmapFontLoader::unload(Resource &resource) {
    if (resource.data) {
        BitmapFontResourceData& data = *static_cast<BitmapFontResourceData *>(resource.data);
        if (data.data.glyphCount > 0 && data.data.glyphs) {
            FF_Memory::ff_free_recursive<FontGlyph>(data.data.glyphs, ARRAY, data.data.glyphCount);
            data.data.glyphs = nullptr;
        }
        if (data.data.kerningCount > 0 && data.data.kernings) {
            FF_Memory::ff_free_recursive<FontKerning>(data.data.kernings, ARRAY, data.data.kerningCount);
            data.data.kernings = nullptr;
        }
        if (data.pageCount > 0 && data.pages) {
            FF_Memory::ff_free_recursive<FontPage>(data.pages, ARRAY, data.pageCount);
            data.pages = nullptr;
        }

        FF_Memory::ff_free<BitmapFontResourceData>(resource.data, memoryTag);
        resource.data = nullptr;
        resource.dataSize = 0;
        resource.loaderId = INVALID_ID_U32;
        resource.path.destroy();
    }
}
