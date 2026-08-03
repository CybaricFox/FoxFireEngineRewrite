//
// Created by cmorg on 7/13/2026.
//

#pragma once
#include "src/defines.h"
#include "foxfire_export.h"

enum FileMode {
    READ = 0x1,
    WRITE = 0x2,
    BOTH = 0x3
};

class FOXFIRE_API FileHandler {
private:
    void* handle = nullptr;
    bool bIsValid = false;

public:
    ~FileHandler();

    bool getFileSize(unsigned long& outSize) const;

    bool exists(const String& name);
    bool openFile(const String& path, FileMode mode, bool isBinary);
    void closeFile();
    bool readLine(String &line, unsigned long maxLength, unsigned long &outLength) const;
    [[nodiscard]] bool writeLine(const String& text) const;
    bool read(unsigned long size, void* outData, unsigned long& outBytesRead) const;
    bool readAll(unsigned char *&outBytes, unsigned long &outBytesRead) const;
    bool readAll(String& outText, unsigned long& outBytesRead) const;
    bool write(unsigned long size, const void* inData, unsigned long& outBytesWritten) const;
};