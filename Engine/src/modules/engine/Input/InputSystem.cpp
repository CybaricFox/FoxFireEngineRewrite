//
// Created by cmorg on 7/1/2026.
//

#include "InputSystem.h"

#include <stack>

#include "../Library/Logger.h"
#include "Keymaps/InputKeymap.h"
#include "src/modules/engine/Memory/FF_Memory.h"

InputSystem* InputSystem::instance = nullptr;

bool InputSystem::isKeyDown(const Keys key) {
    return instance->keyboardState[key] == true;
}

bool InputSystem::isKeyUp(const Keys key) const {
    return keyboardState[key] == false;
}

bool InputSystem::wasKeyDown(const Keys key) const {
    return previousKeyboardState[key] == true;
}

bool InputSystem::wasKeyUp(const Keys key) const {
    return previousKeyboardState[key] == false;
}

bool InputSystem::isButtonDown(const Buttons button) const {
    return mouseButtons[button] == true;
}

bool InputSystem::isButtonUp(const Buttons button) const {
    return mouseButtons[button] == false;
}

void InputSystem::pushKeymap(Keymap* map) {
    if (map == nullptr) {
        Logger::logError("Cannot push a null keymap into the input system.");
        return;
    }
    if (!instance->keymaps.push(map)) {
        Logger::logError("Failed to push keymap");
    }
}

void InputSystem::pullKeymap() {
    if (!instance->keymaps.pull()) {
        Logger::logError("Failed to pull keymap");
    }
}

unsigned int InputSystem::getRequiredFlags() {
    unsigned int requiredFlags = 0;
    if (isKeyDown(KEY_SHIFT) || isKeyDown(KEY_LSHIFT) || isKeyDown(KEY_RSHIFT)) {
        requiredFlags |= KEYMAP_MODIFIER_SHIFT;
    }
    if (isKeyDown(KEY_LALT) || isKeyDown(KEY_RALT)) {
        requiredFlags |= KEYMAP_MODIFIER_ALT;
    }
    if (isKeyDown(KEY_CONTROL) || isKeyDown(KEY_LCONTROL) || isKeyDown(KEY_RCONTROL)) {
        requiredFlags |= KEYMAP_MODIFIER_CONTROL;
    }

    return requiredFlags;
}

bool InputSystem::wasButtonDown(const Buttons button) const {
    return previousMouseButtons[button] == true;
}

bool InputSystem::wasButtonUp(const Buttons button) const {
    return previousMouseButtons[button] == false;
}

void InputSystem::getMousePosition(int& x, int& y) const {
    x = mouseX;
    y = mouseY;
}

void InputSystem::getPreviousMousePosition(int& x, int& y) const {
    x = previousMouseX;
    y = previousMouseY;
}

void InputSystem::update(double deltaTime) {
    //Handle hold bindings
    const unsigned int requiredFlags = getRequiredFlags();
    for (unsigned int i = 0; i < MAX_KEYS; i++) {
        const auto key = static_cast<Keys>(i);

        if (isKeyDown(key) && wasKeyDown(key)) {
            for (int j = keymaps.getSize() - 1; j >= 0; j--) {
                if (keymaps.getData()[j]->processHold(requiredFlags, key)) break;
            }
        }
    }

    //copy current states to previous states
    FF_Memory::ff_copy(previousKeyboardState, keyboardState, sizeof(keyboardState));
    FF_Memory::ff_copy(previousMouseButtons, mouseButtons, sizeof(mouseButtons));
}

void InputSystem::initialize(EngineEvents *engineEventsRef) {
    instance = this;

    keymaps.initialize();
    InputKeymap::initialize();

    engineEventsSystemRef = engineEventsRef;

    Logger::logInfo("Input subsystem initialized!");
}

void InputSystem::shutdown() {
    engineEventsSystemRef = nullptr;
    instance = nullptr;

    keymaps.getData()[0]->destroy();
    FF_Memory::ff_free<Keymap>(keymaps.getData()[0], GAME);
    InputKeymap::destroy();

    keymaps.shutdown();
}

void InputSystem::processKey(const Keys key, const bool bIsPressed) {
    //Only handle this if the key changed
    if (keyboardState[key] == bIsPressed) return;

    keyboardState[key] = bIsPressed;

    //Check keymaps first
    const unsigned int requiredFlags = getRequiredFlags();
    for (int i = keymaps.getSize() - 1; i >= 0; i--) {
        if (keymaps.getData()[i]->processBindings(bIsPressed, requiredFlags, key)) break;
    }

    //Now fire basic input events
    EngineInputContext context{};
    context.key = key;
    engineEventsSystemRef->callEvent(bIsPressed ? KEY_PRESSED : KEY_RELEASED, context);
}

void InputSystem::processButton(const Buttons button, const bool bIsPressed) {
    //Only handle this if the key changed
    if (mouseButtons[button] == bIsPressed) return;

    mouseButtons[button] = bIsPressed;
    EngineInputContext context{};
    context.key = button;
    engineEventsSystemRef->callEvent(bIsPressed ? BUTTON_PRESSED : BUTTON_RELEASED, context);
}

void InputSystem::processMouseMove(const short x, const short y) {
    if (mouseX == x && mouseY == y) return;

    mouseX = x;
    mouseY = y;
    EngineInputContext context{};
    context.mouseX = x;
    context.mouseY = y;
    engineEventsSystemRef->callEvent(MOUSE_MOVED, context);
}

void InputSystem::processMouseScroll(const char z) const {
    EngineInputContext context{};
    context.mouseZ = z;
    engineEventsSystemRef->callEvent(MOUSE_WHEEL, context);
}
