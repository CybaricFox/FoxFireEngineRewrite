//
// Created by cmorg on 7/1/2026.
//

#pragma once

#include "EngineEvents.h"
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
    bool keyboardState[256]{};
    bool previousKeyboardState[256]{};

    //mouse data
    short mouseX = 0;
    short mouseY = 0;
    unsigned char mouseButtons[MAX_BUTTONS]{};
    short previousMouseX = 0;
    short previousMouseY = 0;
    unsigned char previousMouseButtons[MAX_BUTTONS]{};

    bool bIsInitialized = false;

protected:
    [[nodiscard]] bool isButtonDown(Buttons button) const;
    [[nodiscard]] bool isButtonUp(Buttons button) const;
    [[nodiscard]] bool wasButtonDown(Buttons button) const;
    [[nodiscard]] bool wasButtonUp(Buttons button) const;

    void getMousePosition(int& x, int& y) const;
    void getPreviousMousePosition(int& x, int& y) const;

public:
    virtual ~IInputSystem();
    IInputSystem() = default;

    void update(double deltaTime);
    void initialize();

    void processKey(Keys key, bool bIsPressed);
    void processButton(Buttons button, bool bIsPressed);
    void processMouseMove(short x, short y);
    void processMouseScroll(char z);

    [[nodiscard]] bool isKeyDown(Keys key) const;
    [[nodiscard]] bool isKeyUp(Keys key) const;
    [[nodiscard]] bool wasKeyDown(Keys key) const;
    [[nodiscard]] bool wasKeyUp(Keys key) const;

    void subscribeToEngineEvent(EngineEventCode code, const std::function<void(EngineInputContext)> &function, const String &id, Keys key = MAX_KEYS);
};