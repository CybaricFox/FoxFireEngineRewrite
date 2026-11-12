//
// Created by cmorg on 11/2/2026.
//

#include "UITextContainerUtils.h"

#include "src/modules/engine/ECS/Engine_Components/UITextContainer.h"

UIText * UITextContainerUtils::addText(UITextContainer& text, const UITextType type, const String &fontName, const unsigned short fontSize, const String &content) {
    UIText* newText = FF_Memory::ff_allocate<UIText>(ECS);
    if (!newText->create(type, fontName, fontSize, content)) {
        Logger::logError("Failed to create UIText in UITextContainer.");
        return nullptr;
    }

    if (text.texts.getCapacity() == 0) {
        text.texts.initialize(1);
    }
    text.texts.push(newText);
    return newText;
}

void UITextContainerUtils::setText(UITextContainer &component, const unsigned int instanceId, const String &newText) {
    UIText* text = getText(component, instanceId);

    if (text == nullptr) {
        Logger::logError("Failed to fetch UIText with instanceId " + toString(instanceId));
        return;
    }

    text->setText(newText);
}

UIText * UITextContainerUtils::getText(UITextContainer& component, const unsigned int instanceId) {
    for (UIText* text : component.texts) {
        if (text->getInstanceId() == instanceId) return text;
    }

    return nullptr;
}
