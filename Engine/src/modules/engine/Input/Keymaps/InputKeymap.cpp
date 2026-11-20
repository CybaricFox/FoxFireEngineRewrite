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
