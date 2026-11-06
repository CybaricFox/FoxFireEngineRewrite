//
// Created by cmorg on 10/31/2026.
//

#include "UIText.h"

#include "src/modules/engine/Renderer/FontSystem.h"
#include "src/modules/engine/Renderer/MasterRenderSystem.h"
#include "src/modules/engine/Renderer/ShaderSystem.h"

InstanceTracker<UIText> UIText::idGenerator{};

void UIText::regenerate() {
    const unsigned int utfLength = text.getSizeInUTF8();
    const unsigned int charLength = text.getSizeInBytes();
    static constexpr ULong vertexPerQuad = 4;
    static constexpr unsigned char indexPerQuad = 6;
    const ULong vertexBufferSize = sizeof(Vertex2d) * vertexPerQuad * utfLength;
    const ULong indexBufferSize = sizeof(unsigned int) * indexPerQuad * utfLength;

    //Resize buffers
    if (vertexBufferSize > vertexBuffer.getTotalSize()) {
        if (!MasterRenderSystem::resizeRenderBuffer(vertexBuffer, vertexBufferSize)) {
            Logger::logError("Failed to resize vertex render buffer");
            return;
        }
    }
    if (indexBufferSize > indexBuffer.getTotalSize()) {
        if (!MasterRenderSystem::resizeRenderBuffer(indexBuffer, indexBufferSize)) {
            Logger::logError("Failed to resize index render buffer");
            return;
        }
    }

    //Generate Text
    float x = 0;
    float y = 0;
    auto vertexBufferData = static_cast<Vertex2d *>(FF_Memory::ff_allocate_raw(vertexBufferSize, ARRAY, alignof(Vertex2d)));
    auto indexBufferData = static_cast<unsigned int *>(FF_Memory::ff_allocate_raw(indexBufferSize, ARRAY, alignof(unsigned int)));

    unsigned int utfCharCount = 0;
    for (unsigned int c = 0; c < charLength; c++) {
        int codepoint = static_cast<unsigned char>(text[c]);

        switch (codepoint) {
            case '\n': {
                x = 0;
                y += static_cast<float>(data->height);
                utfCharCount++;
                continue;
            }
            case '\t': {
                x += data->tabAdvance;
                utfCharCount++;
                continue;
            }
            default: break;
        }

        unsigned char advance = 0;
        if (!text.getCodepoint(c, codepoint, advance)) {
            Logger::logWarn("Invalid UTF-8 character in UI Text!");
            codepoint = -1;
        }

        //This should probably be made into a binary search
        const FontGlyph* glyph = nullptr;
        for (unsigned int i = 0; i < data->glyphCount; i++) {
            if (data->glyphs[i].codepoint == codepoint) {
                glyph = &data->glyphs[i];
                break;
            }
        }

        if (!glyph) {
            codepoint = -1;
            for (unsigned int i = 0; i < data->glyphCount; i++) {
                if (data->glyphs[i].codepoint == codepoint) {
                    glyph = &data->glyphs[i];
                    break;
                }
            }
        }

        if (!glyph) {
            Logger::logError("Font does not contain a codepoint for -1!");
            utfCharCount++;
            continue;
        }

        const float minX = x + static_cast<float>(glyph->xOffset);
        const float minY = y + static_cast<float>(glyph->yOffset);
        const float maxX = minX + static_cast<float>(glyph->width);
        const float maxY = minY + static_cast<float>(glyph->height);
        const float tMinX = static_cast<float>(glyph->x) / static_cast<float>(data->atlasSizeX);
        const float tMaxX = static_cast<float>(glyph->x + glyph->width) / static_cast<float>(data->atlasSizeX);
        float tMinY = static_cast<float>(glyph->y) / static_cast<float>(data->atlasSizeY);
        float tMaxY = static_cast<float>(glyph->y + glyph->height) / static_cast<float>(data->atlasSizeY);

        //Flip the y axis for system fonts
        if (type == UI_TEXT_TYPE_SYSTEM) {
            tMinY = 1.0f - tMinY;
            tMaxY = 1.0f - tMaxY;
        }

        Vertex2d p0{};
        p0.position = {minX, minY};
        p0.textureCoordinate = {tMinX, tMinY};
        Vertex2d p1{};
        p1.position = {maxX, minY};
        p1.textureCoordinate = {tMaxX, tMinY};
        Vertex2d p2{};
        p2.position = {maxX, maxY};
        p2.textureCoordinate = {tMaxX, tMaxY};
        Vertex2d p3{};
        p3.position = {minX, maxY};
        p3.textureCoordinate = {tMinX, tMaxY};

        vertexBufferData[(utfCharCount * 4) + 0] = p0;
        vertexBufferData[(utfCharCount * 4) + 1] = p2;
        vertexBufferData[(utfCharCount * 4) + 2] = p3;
        vertexBufferData[(utfCharCount * 4) + 3] = p1;

        //Find Kernings if any
        float kerning = 0;
        unsigned int offset = c + advance;
        if (offset < utfLength - 1) {
            int next = 0;
            unsigned char nextAdvance = 0;

            if (!text.getCodepoint(offset, next, nextAdvance)) {
                Logger::logWarn("Invalid UTF-8 character in UI Text!");
                codepoint = -1;
            } else {
                for (unsigned int i = 0; i < data->kerningCount; i++) {
                    FontKerning* k = &data->kernings[i];
                    if (k->codepointA == codepoint && k->codepointB == next) {
                        kerning = k->amount;
                    }
                }
            }
        }

        x += glyph->advance + kerning;

        indexBufferData[(utfCharCount * 6) + 0] = (utfCharCount * 4) + 2;
        indexBufferData[(utfCharCount * 6) + 1] = (utfCharCount * 4) + 1;
        indexBufferData[(utfCharCount * 6) + 2] = (utfCharCount * 4) + 0;
        indexBufferData[(utfCharCount * 6) + 3] = (utfCharCount * 4) + 3;
        indexBufferData[(utfCharCount * 6) + 4] = (utfCharCount * 4) + 0;
        indexBufferData[(utfCharCount * 6) + 5] = (utfCharCount * 4) + 1;

        //Advance c by advance instead of the for loops increment.
        c += advance - 1;
        utfCharCount++;
    }

    bool vertexResult = MasterRenderSystem::loadRenderBufferRange(vertexBuffer, 0, vertexBufferSize, vertexBufferData);
    bool indexResult = MasterRenderSystem::loadRenderBufferRange(indexBuffer, 0, indexBufferSize, indexBufferData);

    FF_Memory::ff_free_raw(vertexBufferData, vertexBufferSize, ARRAY, alignof(Vertex2d));
    FF_Memory::ff_free_raw(indexBufferData, indexBufferSize, ARRAY, alignof(unsigned int));

    //Verify results
    if (!vertexResult) {
        Logger::logError("UIText regeneration failed to regenerate vertices!");
    }
    if (!indexResult) {
        Logger::logError("UIText regeneration failed to regenerate indices!");
    }
}

bool UIText::create(const UITextType newType, const String &fontName, const unsigned short fontSize, const String &content) {
    if (fontName.isEmpty() || content.isEmpty()) {
        Logger::logError("UIText must have a valid font name and content.");
        return false;
    }

    type = newType;

    //Setup Font
    if (!FontSystem::acquireFont(fontName, fontSize, *this)) {
        Logger::logError("UIText failed to acquire font " + fontName);
        return false;
    }

    text = content;
    static constexpr ULong quadSize = sizeof(Vertex2d) * 4;

    unsigned int textLength = text.getSizeInBytes();
    if (textLength < 1) {
        textLength = 1;
    }

    //Setup shader resources
    const Shader& uiShader = *ShaderSystem::getShader("Fox_Fire_UI_Shader");
    TextureMap* fontMaps[1] = {&data->atlas};

    if (!MasterRenderSystem::acquireInstanceResources(uiShader, instanceId, fontMaps)) {
        Logger::logFatal("Failed to acquire shader resource for UI Text.");
        return false;
    }

    //Setup vertex buffer
    if (!MasterRenderSystem::createRenderBuffer(RENDERBUFFER_TYPE_VERTEX, textLength * quadSize, false, vertexBuffer)) {
        Logger::logError("Failed to create vertex buffer for UIText.");
        return false;
    }
    if (!MasterRenderSystem::bindRenderBuffer(vertexBuffer, 0)) {
        Logger::logError("Failed to bind vertex buffer for UIText.");
        return false;
    }

    //Setup index buffer
    static constexpr unsigned char quadIndexSize = sizeof(unsigned int) * 6;
    if (!MasterRenderSystem::createRenderBuffer(RENDERBUFFER_TYPE_INDEX, textLength * quadIndexSize, false, indexBuffer)) {
        Logger::logError("Failed to create index buffer for UIText.");
        return false;
    }
    if (!MasterRenderSystem::bindRenderBuffer(indexBuffer, 0)) {
        Logger::logError("Failed to bind index buffer for UIText.");
        return false;
    }

    //Verify atlas
    if (!FontSystem::verifyAtlas(*data, content)) {
        Logger::logError("Failed to verify atlas.");
        return false;
    }

    regenerate();

    id = idGenerator.acquireId(this);

    return true;
}

void UIText::destroy() {
    idGenerator.freeId(id);

    MasterRenderSystem::destroyRenderBuffer(vertexBuffer);
    MasterRenderSystem::destroyRenderBuffer(indexBuffer);

    const Shader& uiShader = *ShaderSystem::getShader("Fox_Fire_UI_Shader");
    if (!MasterRenderSystem::releaseInstanceResources(uiShader, instanceId)) {
        Logger::logFatal("Failed to release shader resource for UIText.");
    }

    text.destroy();
}

void UIText::shutdown() {
    idGenerator.shutdown();
}

void UIText::setText(const String &newText) {
    text = newText;

    if (!FontSystem::verifyAtlas(*data, text)) {
        Logger::logError("Failed to verify atlas.");
    }

    regenerate();
}

void UIText::draw() {
    static constexpr ULong quadVertexCount = 4;
    if (!MasterRenderSystem::drawRenderBuffer(vertexBuffer, 0, text.getSizeInBytes() * quadVertexCount, true)) {
        Logger::logError("Failed to draw vertex buffer for UIText.");
    }

    static constexpr unsigned char quadIndexCount = 6;
    if (!MasterRenderSystem::drawRenderBuffer(indexBuffer, 0, text.getSizeInBytes() * quadIndexCount, false)) {
        Logger::logError("Failed to draw index buffer for UIText.");
    }
}