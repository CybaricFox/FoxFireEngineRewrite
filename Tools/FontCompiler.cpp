//
// Created by cmorg on 10/6/2026.
//

#include "FontCompiler.h"

#include <cstring>
#include <iostream>
#include <ostream>

#include "FileHandler.h"
#include "StringUtils.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>


void FontCompiler::compileFont(const std::string &filePath, const std::string &outputFolder) {
    constexpr int FIRST_GLYPH = 32;
    constexpr int LAST_GLYPH = 126;
    constexpr int GLYPH_COUNT = LAST_GLYPH - FIRST_GLYPH + 1;

    FontData outFont{};

    FileHandler ttf{};
    FileHandler foxFont{};

    if(!ttf.openFile(filePath + ".ttf", READ, true)) {
        ttf.closeFile();
        return;
    }

    if(!foxFont.openFile(filePath + ".FoxFont", READ, false)) {
        ttf.closeFile();
        foxFont.closeFile();
        return;
    }

    readFoxFont(foxFont, outFont);
    foxFont.closeFile();
    if (!foxFont.openFile(outputFolder + "/" + outFont.fontName + ".FoxFont", WRITE, false)) {
        std::cout << "Failed to write file: " << outputFolder + "/" + outFont.fontName + ".FoxFont" << std::endl;
        ttf.closeFile();
        foxFont.closeFile();
        return;
    }

    ULong ttfSize = 0;
    ULong bytesRead = 0;
    ttf.getFileSize(ttfSize);
    auto pixels = static_cast<unsigned char *>(malloc(ttfSize));
    auto bitMap = static_cast<unsigned char *>(malloc(outFont.targetWidth * outFont.targetHeight));

    if (!ttf.read(ttfSize, pixels, bytesRead)) {
        std::cout << "Failed to read ttf file: " << filePath + ".ttf" << std::endl;
        foxFont.closeFile();
        ttf.closeFile();
        free(pixels);
        return;
    }

    stbtt_packedchar glyphs[GLYPH_COUNT]{};
    stbtt_pack_context context{};

    if (!stbtt_PackBegin(&context, bitMap,static_cast<int>(outFont.targetWidth),static_cast<int>(outFont.targetHeight),0,1,nullptr)) {
        std::cout << "Failed to initialize font atlas." << std::endl;

        ttf.closeFile();
        foxFont.closeFile();
        free(pixels);
        free(bitMap);
        return;
    }

    stbtt_PackSetOversampling(&context,1,1);

    const int result = stbtt_PackFontRange(&context, pixels,0,static_cast<float>(outFont.targetFontSize),FIRST_GLYPH,GLYPH_COUNT,glyphs);

    stbtt_PackEnd(&context);

    if (!result) {
        std::cout
            << "Font atlas is too small to contain all glyphs."
            << std::endl;

        ttf.closeFile();
        foxFont.closeFile();
        free(pixels);
        free(bitMap);
        return;
    }

    //Convert to RGBA
    auto rgbMap = static_cast<unsigned char *>(malloc(outFont.targetWidth * outFont.targetHeight * 4));

    for (ULong i = 0; i < outFont.targetWidth * outFont.targetHeight; ++i) {
        const unsigned char coverage = bitMap[i];

        rgbMap[(i * 4) + 0] = 255;
        rgbMap[(i * 4) + 1] = 255;
        rgbMap[(i * 4) + 2] = 255;
        rgbMap[(i * 4) + 3] = coverage;
    }

    //Write the png
    const std::string pngPath = outputFolder + "/" + outFont.engineName + ".png";

    if (!stbi_write_png(
        pngPath.c_str(),
        static_cast<int>(outFont.targetWidth),
        static_cast<int>(outFont.targetHeight),
        4,
        rgbMap,
        static_cast<int>(outFont.targetWidth) * 4
    )) {
        std::cout
            << "Failed to write font atlas: "
            << pngPath
            << std::endl;

        ttf.closeFile();
        foxFont.closeFile();
        free(pixels);
        free(bitMap);
        free(rgbMap);

        return;
    }

    //Write header info
    foxFont.writeLine("Font = " + outFont.fontName);
    foxFont.writeLine("Name = " + outFont.engineName);
    foxFont.writeLine("Size = " + std::to_string(outFont.targetFontSize));
    foxFont.writeLine("Width = " + std::to_string(outFont.targetWidth));
    foxFont.writeLine("Height = " + std::to_string(outFont.targetHeight));
    foxFont.writeLine("Glyph_Count = " + std::to_string(GLYPH_COUNT));

    stbtt_fontinfo tempFontInfo{};
    const int tempOffset = stbtt_GetFontOffsetForIndex(pixels, 0);
    if (tempOffset < 0 || !stbtt_InitFont(&tempFontInfo, pixels, tempOffset)) {
        std::cerr << "Failed to initialize temp font info." << std::endl;
        return;
    }
    writeKerningCount(tempFontInfo, foxFont);

    foxFont.writeLine("Page_Count = 1");

    //metadata
    for (int i = 0; i < GLYPH_COUNT; ++i) {
        const int codepoint = FIRST_GLYPH + i;

        const stbtt_packedchar& glyph = glyphs[i];

        const int width = glyph.x1 - glyph.x0;

        const int height = glyph.y1 - glyph.y0;

        foxFont.writeLine("Glyph = " + std::to_string(codepoint) +
            ", Page = 0" +
            ", X = " + std::to_string(glyph.x0) +
            ", Y = " + std::to_string(glyph.y0) +
            ", Width = " + std::to_string(width) +
            ", Height = " + std::to_string(height) +
            ", X_Offset = " + std::to_string(glyph.xoff) +
            ", Y_Offset = " + std::to_string(glyph.yoff) +
            ", Advance = " + std::to_string(glyph.xadvance)
        );
    }
    std::cout << "Compiled font: " << outFont.engineName << std::endl;

    //Now create the kernings
    stbtt_fontinfo fontInfo{};
    const int offset = stbtt_GetFontOffsetForIndex(pixels, 0);
    if (offset < 0 || !stbtt_InitFont(&fontInfo, pixels, offset)) {
        std::cerr << "Failed to initialize font info." << std::endl;
        return;
    }

    //This is not as optimized as it could be, but since this is an offline tool, it doesn't matter as much.
    //Worse case scenario, refactor this code and the kerning code.
    createKernings(fontInfo, outFont, foxFont);

    ttf.closeFile();
    foxFont.closeFile();
    free(pixels);
    free(bitMap);
    free(rgbMap);
}

void FontCompiler::readFoxFont(const FileHandler &file, FontData &fontData) {
    String line{};
    ULong bytesRead = 0;

    while (file.readLine(line, 512, bytesRead)) {
        char regex = '=';

        const ULong index = line.find(regex);

        if (index == String::npos) {
            continue;
        }

        String variable = line.substr(0, index);
        String value = line.substr(index + 1);

        StringUtils::trim(variable);
        StringUtils::trim(value);

        if (strcmp(variable.c_str(), "Font") == 0) {
            fontData.fontName = value;
            continue;
        }
        if (strcmp(variable.c_str(), "Name") == 0) {
            fontData.engineName = value;
            continue;
        }
        if (strcmp(variable.c_str(), "Size") == 0) {
            StringUtils::stringToUInt(value, fontData.targetFontSize);
            continue;
        }
        if (strcmp(variable.c_str(), "Width") == 0) {
            StringUtils::stringToUInt(value, fontData.targetWidth);
            continue;
        }
        if (strcmp(variable.c_str(), "Height") == 0) {
            StringUtils::stringToUInt(value, fontData.targetHeight);
            continue;
        }
    }
}

void FontCompiler::createKernings(const stbtt_fontinfo& fontInfo, const FontData& fontData, const FileHandler& file) {
    //Duplicate code here
    constexpr int FIRST_GLYPH = 32;
    constexpr int LAST_GLYPH = 126;

    const float scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(fontData.targetFontSize));

    int kerningCount = 0;

    for (int first = FIRST_GLYPH; first <= LAST_GLYPH; ++first) {
        for (int second = FIRST_GLYPH; second <= LAST_GLYPH; ++second) {
            const int advance = stbtt_GetCodepointKernAdvance(&fontInfo, first, second);

            if (advance == 0) {
                continue;
            }

            const float scaledAdvance = static_cast<float>(advance) * scale;

            file.writeLine("Kerning = " + std::to_string(first) +
                ", Next = " + std::to_string(second) +
                ", Advance = " + std::to_string(scaledAdvance)
            );

            ++kerningCount;
        }
    }

    std::cout << "Kerning pairs: " << kerningCount << std::endl;
}

void FontCompiler::writeKerningCount(const stbtt_fontinfo &fontInfo, const FileHandler &file) {
    //Duplicate code here
    constexpr int FIRST_GLYPH = 32;
    constexpr int LAST_GLYPH = 126;

    int kerningCount = 0;

    for (int first = FIRST_GLYPH; first <= LAST_GLYPH; ++first) {
        for (int second = FIRST_GLYPH; second <= LAST_GLYPH; ++second) {
            const int advance = stbtt_GetCodepointKernAdvance(&fontInfo, first, second);

            if (advance == 0) {
                continue;
            }

            ++kerningCount;
        }
    }

    file.writeLine("Kerning_Count = " + std::to_string(kerningCount));
}
