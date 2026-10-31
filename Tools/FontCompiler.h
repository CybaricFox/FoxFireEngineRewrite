//
// Created by cmorg on 10/6/2026.
//

#pragma once
#include <string>

#include "FileHandler.h"
#include "stb/stb_truetype.h"

struct FontData {
    //Input data
    String fontName{};
    String engineName{};
    unsigned int targetFontSize = 0;
    unsigned int targetWidth = 0;
    unsigned int targetHeight = 0;
};

class FontCompiler {
private:
    void compileFont(const std::string& filePath, const std::string& outputFolder);
    void readFoxFont(const FileHandler& file, FontData& fontData);
    void createKernings(const stbtt_fontinfo &fontInfo, const FontData &fontData, const FileHandler &file);
    void writeKerningCount(const stbtt_fontinfo &fontInfo, const FileHandler &file);

public:
    FontCompiler(const std::string& filePath, const std::string& outputFolder) {
        compileFont(filePath, outputFolder);
    };
};