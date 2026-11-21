//
// Created by cmorg on 11/20/2026.
//

#include "InputKeymap.h"

#include "src/modules/engine/Input/InputSystem.h"

Keymap InputKeymap::inputKeymap{};
IInputConsumer* InputKeymap::inputConsumer = nullptr;

void InputKeymap::initialize() {
    inputKeymap.setOverride(true);

    KeymapCallback callback = []KeymapLambda{inputConsumer->onInput(key, modifierFlags);};

    for (unsigned int i = 0; i < MAX_KEYS; i++) {
        inputKeymap.addBinding(static_cast<Keys>(i), KEYMAP_BIND_PRESS, 0, nullptr, callback, "InputKeymap." + toString(i));
    }

    KeymapCallback scrollUp = []KeymapLambda{inputConsumer->scrollUp(modifierFlags);};
    KeymapCallback scrollDown = []KeymapLambda{inputConsumer->scrollDown(modifierFlags);};
    KeymapCallback historyUp = []KeymapLambda{inputConsumer->historyUp(modifierFlags);};
    KeymapCallback historyDown = []KeymapLambda{inputConsumer->historyDown(modifierFlags);};

    inputKeymap.addBinding(KEY_UP, KEYMAP_BIND_HOLD, 0, nullptr, scrollUp, "InputKeymap.Up");
    inputKeymap.addBinding(KEY_DOWN, KEYMAP_BIND_HOLD, 0, nullptr, scrollDown, "InputKeymap.Down");
    inputKeymap.addBinding(KEY_UP, KEYMAP_BIND_PRESS, KEYMAP_MODIFIER_SHIFT, nullptr, historyUp, "InputKeymap.HistoryUp");
    inputKeymap.addBinding(KEY_DOWN, KEYMAP_BIND_PRESS, KEYMAP_MODIFIER_SHIFT, nullptr, historyDown, "InputKeymap.HistoryDown");
}

void InputKeymap::destroy() {
    inputKeymap.destroy();
}

void InputKeymap::activate(IInputConsumer *consumer) {
    inputConsumer = consumer;

    InputSystem::pushKeymap(&inputKeymap);
}

void InputKeymap::deactivate() {
    InputSystem::pullKeymap();

    inputConsumer = nullptr;
}
