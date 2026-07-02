//
// Created by cmorg on 7/1/2026.
//

#pragma once

#include "EngineEvents.h"
#include "../Library/FF_Memory.h"
#include "foxfire_export.h"

enum Buttons{
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_MIDDLE,
    MAX_BUTTONS
};

class FOXFIRE_API IInputSystem {
private:
    //Keyboard data
    bool keyboardState[256];
    bool previousKeyboardState[256];

    //mouse data
    short mouseX = 0;
    short mouseY = 0;
    unsigned char mouseButtons[MAX_BUTTONS];
    short previousMouseX = 0;
    short previousMouseY = 0;
    unsigned char previousMouseButtons[MAX_BUTTONS];

    bool bIsInitialized = false;

protected:
    bool isKeyDown(Keys key);
    bool isKeyUp(Keys key);
    bool wasKeyDown(Keys key);
    bool wasKeyUp(Keys key);
    bool isButtonDown(Buttons button);
    bool isButtonUp(Buttons button);
    bool wasButtonDown(Buttons button);
    bool wasButtonUp(Buttons button);

    void getMousePosition(int *x, int *y);
    void getPreviousMousePosition(int *x, int *y);

public:
    ~IInputSystem();

    void update(double deltaTime, FF_Memory& ff_memory);
    void initialize(FF_Memory& ff_memory);

    void processKey(Keys key, bool bIsPressed);
    void processButton(Buttons button, bool bIsPressed);
    void processMouseMove(short x, short y);
    void processMouseScroll(char z);

    void subscribeToEngineEvent(EngineEventCode code, const std::function<void(EngineInputContext)> &function, const String &id, Keys key = MAX_KEYS);
};