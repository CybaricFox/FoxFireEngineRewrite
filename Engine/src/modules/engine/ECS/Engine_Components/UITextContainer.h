//
// Created by cmorg on 11/2/2026.
//

#pragma once
#include "src/modules/engine/ECS/ECSTypes.h"
#include "src/modules/engine/UserInterface/UIText.h"

struct UITextContainer final : EntityComponentWrapper<UITextContainer> {
    DynamicArray<UIText*> texts{};

    UITextContainer() = default;
    ~UITextContainer() override {
        for (UIText* text : texts) {
            text->destroy();
            FF_Memory::ff_free<UIText>(text, ECS);
        }

        texts.shutdown();
    }

    UITextContainer(const UITextContainer& other) {
        if (!other.texts.isEmpty()) {
            texts.initialize(other.texts.getLength());

            for (UIText* text : other.texts) {
                texts.push(text);
            }
        }
    }
};
