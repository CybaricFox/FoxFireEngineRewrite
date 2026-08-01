//
// Created by cmorg on 7/13/2026.
//

#include "FileHandler.h"

#include <cstring>
#include <filesystem>

#include "Logger.h"
#include "src/modules/engine/Memory/FF_Memory.h"

FileHandler::~FileHandler() {
    closeFile();
}

bool FileHandler::exists(const String &name) {
    return std::filesystem::exists(name);
}

bool FileHandler::openFile(const String &path, const FileMode mode, const bool isBinary) {
    bIsValid = false;
    handle = nullptr;

    String fileMode;

    switch (mode) {
        case READ: {
            fileMode = isBinary ? "rb" : "r";
            break;
        }
        case WRITE: {
            fileMode = isBinary ? "wb" : "w";
            break;
        }
        case BOTH: {
            fileMode = isBinary ? "w+b" : "w+";
            break;
        }
    }

    FILE* file = fopen(path.c_str(), fileMode.c_str());
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
        fclose(static_cast<FILE *>(handle));
        handle = nullptr;
        bIsValid = false;
    }
}

bool FileHandler::readLine(String& line, const unsigned long maxLength, unsigned long& outLength) const {
    if (!handle || maxLength == 0) return false;

    //ensures string is the size of the line being read.
    line.resize(maxLength);

    if (fgets(line.data(), static_cast<int>(maxLength), static_cast<FILE *>(handle)) != nullptr) {
        outLength = std::strlen(line.data());
        line.resize(outLength);
        return true;
    }

    return false;
}

bool FileHandler::writeLine(const String &text) const {
    if (handle) {
        int result = fputs(text.c_str(), static_cast<FILE *>(handle));
        if (result != EOF) {
            result = fputc('\n', static_cast<FILE *>(handle));
        }

        //If the program crashes and we dont flush, then the file will not save.
        fflush(static_cast<FILE *>(handle));
        return result != EOF;
    }

    return false;
}

bool FileHandler::read(const unsigned long size, void *outData, unsigned long &outBytesRead) {
    if (!handle || !outData) return false;

    outBytesRead = fread(outData, 1, size, static_cast<FILE *>(handle));
    if (outBytesRead != size) {
        return false;
    }

    return true;
}

bool FileHandler::readAll(unsigned char *&outBytes, unsigned long &outBytesRead) {
    if (!handle) return false;

    fseek(static_cast<FILE *>(handle), 0, SEEK_END);
    const unsigned long size = ftell(static_cast<FILE *>(handle));
    rewind(static_cast<FILE *>(handle));

    outBytes = static_cast<unsigned char *>(FF_Memory::ff_allocate(sizeof(unsigned char) * size, CHAR_ARRAY));
    outBytesRead = fread(outBytes, 1, size, static_cast<FILE *>(handle));
    if (outBytesRead != size) return false;

    return true;
}

bool FileHandler::write(const unsigned long size, const void *inData, unsigned long &outBytesWritten) {
    if (!handle) return false;

    outBytesWritten = fwrite(inData, 1, size, static_cast<FILE *>(handle));
    if (outBytesWritten != size) {
        return false;
    }
    fflush(static_cast<FILE *>(handle));

    return true;
}
