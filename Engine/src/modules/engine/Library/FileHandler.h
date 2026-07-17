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

    bool exists(const String& name);
    bool openFile(const String& path, FileMode mode, bool isBinary);
    void closeFile();
    bool readLine(String*& line);
    bool writeLine(const String& text);
    bool read(unsigned long size, void* outData, unsigned long& outBytesRead);
    bool readAll(unsigned char*& outBytes, unsigned long& outBytesRead);
    bool write(unsigned long size, const void* inData, unsigned long& outBytesWritten);
};