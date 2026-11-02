//
// Created by cmorg on 10/31/2026.
//

#pragma once
#include "src/modules/engine/ECS/Engine_Components/Transform.h"
#include "src/modules/engine/Renderer/RenderBuffer.h"
#include "src/modules/engine/Resources/FontTypes.h"

/**
 *  @file UIText.h
 *  @layer Engine
 *  @module UserInterface
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 10/31/2026
 *
 *  @copyright (c) 2026
 */

enum UITextType {
    UI_TEXT_TYPE_BITMAP,
    UI_TEXT_TYPE_SYSTEM
};

class UIText {
private:
    UITextType type{};
    FontData* data = nullptr;
    RenderBuffer vertexBuffer{};
    RenderBuffer indexBuffer{};
    String text{};
    Transform transform{};
    unsigned int instanceId = INVALID_ID_U32;
    ULong frameNumber = INVALID_ID_U64;

    void regenerate();

public:
    bool create(UITextType newType, const String &fontName, unsigned short fontSize, const String &content);
    void destroy();

    [[nodiscard]] UITextType getType() const {return type;}
    [[nodiscard]] String getFontName() const {
        if (data == nullptr) return {};
        return data->name;
    }
    [[nodiscard]] unsigned int getInstanceId() const {return instanceId;}
    [[nodiscard]] TextureMap& getTextureAtlas() const {return data->atlas;}
    [[nodiscard]] ULong getFrameNumber() const {return frameNumber;}
    Transform& getTransform() {return transform;}

    void setPosition(const Vector3f position) {transform.position = position;}
    void setText(const String &newText);
    void setFont(FontData* fontData) {data = fontData;}
    void setFrameNumber(const ULong newFrameNumber) {frameNumber = newFrameNumber;}

    void draw();
};