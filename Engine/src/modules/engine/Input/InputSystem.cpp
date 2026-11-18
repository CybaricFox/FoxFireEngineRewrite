//
// Created by cmorg on 7/1/2026.
//

#include "InputSystem.h"

#include "../Library/Logger.h"
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
    //copy current states to previous states
    FF_Memory::ff_copy(previousKeyboardState, keyboardState, sizeof(keyboardState));
    FF_Memory::ff_copy(previousMouseButtons, mouseButtons, sizeof(mouseButtons));
}

void InputSystem::initialize(EngineEvents *engineEventsRef) {
    instance = this;

    FF_Memory::ff_clear(keyboardState, sizeof(keyboardState));
    FF_Memory::ff_clear(previousKeyboardState, sizeof(previousKeyboardState));
    FF_Memory::ff_clear(mouseButtons, sizeof(mouseButtons));
    FF_Memory::ff_clear(previousMouseButtons, sizeof(previousMouseButtons));

    engineEventsSystemRef = engineEventsRef;

    Logger::logInfo("Input subsystem initialized!");
}

void InputSystem::shutdown() {
    engineEventsSystemRef = nullptr;
    instance = nullptr;
}

void InputSystem::processKey(const Keys key, const bool bIsPressed) {
    //Only handle this if the key changed
    if (keyboardState[key] == bIsPressed) return;

    keyboardState[key] = bIsPressed;

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
