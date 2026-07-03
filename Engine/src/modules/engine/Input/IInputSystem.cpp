//
// Created by cmorg on 7/1/2026.
//

#include "IInputSystem.h"

#include "../Library/Logger.h"

bool IInputSystem::isKeyDown(const Keys key) {
    if (!bIsInitialized) return false;
    return keyboardState[key] == true;
}

bool IInputSystem::isKeyUp(const Keys key) {
    if (!bIsInitialized) return false;
    return keyboardState[key] == false;
}

bool IInputSystem::wasKeyDown(const Keys key) {
    if (!bIsInitialized) return false;
    return previousKeyboardState[key] == true;
}

bool IInputSystem::wasKeyUp(const Keys key) {
    if (!bIsInitialized) return false;
    return previousKeyboardState[key] == false;
}

bool IInputSystem::isButtonDown(const Buttons button) {
    if (!bIsInitialized) return false;
    return mouseButtons[button] == true;
}

bool IInputSystem::isButtonUp(const Buttons button) {
    if (!bIsInitialized) return false;
    return mouseButtons[button] == false;
}

bool IInputSystem::wasButtonDown(const Buttons button) {
    if (!bIsInitialized) return false;
    return previousMouseButtons[button] == true;
}

bool IInputSystem::wasButtonUp(const Buttons button) {
    if (!bIsInitialized) return false;
    return previousMouseButtons[button] == false;
}

void IInputSystem::getMousePosition(int *x, int *y) {
    if (!bIsInitialized) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = mouseX;
    *y = mouseY;
}

void IInputSystem::getPreviousMousePosition(int *x, int *y) {
    if (!bIsInitialized) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = previousMouseX;
    *y = previousMouseY;
}

IInputSystem::~IInputSystem() {
    bIsInitialized = false;
}

void IInputSystem::update(double deltaTime) {
    if (!bIsInitialized) {
        return;
    }

    //copy current states to previous states
    FF_Memory::ff_copy(previousKeyboardState, keyboardState, sizeof(keyboardState));
    FF_Memory::ff_copy(previousMouseButtons, mouseButtons, sizeof(mouseButtons));
}

void IInputSystem::initialize() {
    FF_Memory::ff_clear(keyboardState, sizeof(keyboardState));
    FF_Memory::ff_clear(previousKeyboardState, sizeof(previousKeyboardState));
    FF_Memory::ff_clear(mouseButtons, sizeof(mouseButtons));
    FF_Memory::ff_clear(previousMouseButtons, sizeof(previousMouseButtons));

    bIsInitialized = true;
    Logger::logInfo("Input subsystem initialized!");
}

void IInputSystem::processKey(const Keys key, const bool bIsPressed) {
    //Only handle this if the key changed
    if (keyboardState[key] == bIsPressed) return;

    keyboardState[key] = bIsPressed;

    EngineInputContext context{};
    context.key = key;
    EngineEvents::callEvent(bIsPressed ? KEY_PRESSED : KEY_RELEASED, context);
}

void IInputSystem::processButton(const Buttons button, const bool bIsPressed) {
    //Only handle this if the key changed
    if (mouseButtons[button] == bIsPressed) return;

    mouseButtons[button] = bIsPressed;
    EngineInputContext context{};
    context.key = button;
    EngineEvents::callEvent(bIsPressed ? BUTTON_PRESSED : BUTTON_RELEASED, context);
}

void IInputSystem::processMouseMove(const short x, const short y) {
    if (mouseX == x && mouseY == y) return;

    mouseX = x;
    mouseY = y;
    EngineInputContext context{};
    context.mouseX = x;
    context.mouseY = y;
    EngineEvents::callEvent(MOUSE_MOVED, context);
}

void IInputSystem::processMouseScroll(const char z) {
    EngineInputContext context{};
    context.mouseZ = z;
    EngineEvents::callEvent(MOUSE_WHEEL, context);
}

void IInputSystem::subscribeToEngineEvent(const EngineEventCode code, const std::function<void(EngineInputContext)>& function, const String &id, const Keys key) {
    EngineEvents::subscribe(code, [callback = function](const EngineInputContext context){callback(context);}, id, key);
}
