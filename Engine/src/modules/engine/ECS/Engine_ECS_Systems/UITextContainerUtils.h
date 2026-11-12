//
// Created by cmorg on 11/2/2026.
//

#pragma once
#include "src/modules/engine/ECS/Engine_Components/UITextContainer.h"
#include "src/modules/engine/UserInterface/UIText.h"

/**
 *  @file UITextContainerUtils.h
 *  @layer Engine
 *  @module ECS
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/2/2026
 *
 *  @copyright (c) 2026
 */

class UITextContainerUtils {
public:
    static UIText* addText(UITextContainer &text, UITextType type, const String &fontName, unsigned short fontSize, const String &content);
    static void setText(UITextContainer &component, unsigned int instanceId, const String &newText);

    static UIText* getText(UITextContainer &component, unsigned int instanceId);
};