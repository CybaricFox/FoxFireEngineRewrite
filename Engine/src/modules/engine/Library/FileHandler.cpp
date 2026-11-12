//
// Created by cmorg on 7/13/2026.
//

#include "FileHandler.h"

#include <cstring>
#include <filesystem>

#include "Logger.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Memory/FF_Memory.h"

bool FileHandler::getFileSize(ULong &outSize) const {
    if (!handle) return false;

    fseek(handle, 0, SEEK_END);
    outSize = ftell(handle);
    rewind(handle);
    return true;
}

String* FileHandler::getAllFiles(const String &filePath, const String &extension, unsigned int &outFileCount) {
    DynamicArray<String> files{0};

    if (filePath.isEmpty()) return nullptr;

    std::error_code error;
    std::filesystem::directory_iterator iterator(
        filePath.getAsCharString(), error
    );

    if (error) {
        Logger::logError("Failed to open directory: " + filePath);
        return nullptr;
    }

    const std::filesystem::directory_iterator end{};

    while (iterator != end) {
        const auto& entry = *iterator;

        const bool isFile = entry.is_regular_file(error);
        if (error) {
            Logger::logError("Failed to inspect directory entry.");
            return nullptr;
        }

        if (isFile && entry.path().extension().string() == extension.getAsCharString()) {
            files.push(entry.path().filename().string().c_str());
        }

        iterator.increment(error);
        if (error) {
            Logger::logError("Failed to iterate directory.");
            return nullptr;
        }
    }

    outFileCount = files.getLength();
    const auto strings = FF_Memory::ff_allocate_recursive<String>(ARRAY, outFileCount);
    for (unsigned int i = 0; i < outFileCount; i++) {
        strings[i] = files[i];
        files[i].destroy();
    }
    files.shutdown();

    return strings;
}

FileHandler::~FileHandler() {
    closeFile();
}

bool FileHandler::exists(const String &name) {
    return std::filesystem::exists(name.getAsCharString());
}

bool FileHandler::openFile(const String &path, const FileMode mode, const bool isBinary) {
    bIsValid = false;
    handle = nullptr;

    String fileMode;

    switch (mode) {
        case READ: {
            if (isBinary) {
                fileMode = "rb";
            } else {
                fileMode = "r";
            }
            break;
        }
        case WRITE: {
            if (isBinary) {
                fileMode = "wb";
            } else {
                fileMode = "w";
            }
            break;
        }
        case BOTH: {
            if (isBinary) {
                fileMode = "w+b";
            } else {
                fileMode = "w+";
            }
            break;
        }
    }

    FILE* file = fopen(path.getAsCharString(), fileMode.getAsCharString());
    if (!file) {
        Logger::logError("Failed to open file: " + path);
        return false;
    }

    handle = file;
    bIsValid = true;

    return true;
}

void FileHandler::closeFile() {
    if (handle) {
        fclose(handle);
        handle = nullptr;
        bIsValid = false;
    }
}

bool FileHandler::readLine(String& line, const unsigned long maxLength, ULong& outLength) const {
    if (!handle || maxLength == 0) return false;

    char buffer[maxLength];
    if (fgets(buffer, static_cast<int>(maxLength), handle) != nullptr) {
        outLength = std::strlen(buffer);
        line.setString(buffer, std::strlen(buffer));
        FF_Memory::ff_clear(buffer, sizeof(char) * maxLength);
        return true;
    }

    return false;
}

bool FileHandler::writeLine(const String &text) const {
    if (handle) {
        int result = fputs(text.getAsCharString(), handle);
        if (result != EOF) {
            result = fputc('\n', handle);
        }

        //If the program crashes and we dont flush, then the file will not save.
        fflush(handle);
        return result != EOF;
    }

    return false;
}

bool FileHandler::read(const unsigned long size, void *outData, ULong &outBytesRead) const {
    if (!handle || !outData) return false;

    outBytesRead = fread(outData, 1, size, handle);
    if (outBytesRead != size) {
        return false;
    }

    return true;
}

bool FileHandler::readAll(unsigned char*& outBytes, ULong &outBytesRead) const {
    if (!handle) return false;

    ULong size = 0;
    getFileSize(size);

    outBytesRead = fread(outBytes, 1, size, handle);
    return outBytesRead == size;
}
bool FileHandler::readAll(String &outText, ULong &outBytesRead) const {
    if (!handle) return false;

    ULong size = 0;
    getFileSize(size);

    outBytesRead = fread(&outText, 1, size, handle);
    return outBytesRead == size;
}

bool FileHandler::write(const unsigned long size, const void *inData, ULong &outBytesWritten) const {
    if (!handle) return false;

    outBytesWritten = fwrite(inData, 1, size, handle);
    if (outBytesWritten != size) {
        return false;
    }
    fflush(handle);

    return true;
}
