/**
*   @file IInputSystem.h
 *  @layer Engine
 *  @module Input
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "EngineEvents.h"
#include "foxfire_export.h"

/**
 * @brief Mouse buttons
 */
enum Buttons{
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_MIDDLE,
    MAX_BUTTONS
};

/**
 * @brief Input interface. Can be used as is if needed. Interfaces the EngineEvent system and handles user input.
 */
class FOXFIRE_API IInputSystem {
private:
    /** @brief Pressed state of each key. True is pressed. False is not pressed. */
    bool keyboardState[256]{};
    /** @brief The state of the key on the last frame. */
    bool previousKeyboardState[256]{};

    /** @brief Current mouse x position */
    short mouseX = 0;
    /** @brief Current mouse y position */
    short mouseY = 0;
    /** @brief Pressed state of each mouse button. */
    unsigned char mouseButtons[MAX_BUTTONS]{};
    /** @brief mouse x position on the last frame*/
    short previousMouseX = 0;
    /** @brief mouse y position on the last frame*/
    short previousMouseY = 0;
    /** @brief Pressed state of each mouse button on the last frame */
    unsigned char previousMouseButtons[MAX_BUTTONS]{};

    bool bIsInitialized = false;
    unsigned long memorySize = 0;

    EngineEvents* engineEventsSystemRef = nullptr;

protected:
    /**
     * @brief
     * @param button The button to check
     * @return True if the button is down.
     */
    [[nodiscard]] bool isButtonDown(Buttons button) const;
    /**
     * @brief
     * @param button The button to check
     * @return True if the button is up.
     */
    [[nodiscard]] bool isButtonUp(Buttons button) const;
    /**
     * @brief
     * @param button The button to check
     * @return True if the button was down on the last frame.
     */
    [[nodiscard]] bool wasButtonDown(Buttons button) const;
    /**
     * @brief
     * @param button The button to check
     * @return True if the button was up on the last frame.
     */
    [[nodiscard]] bool wasButtonUp(Buttons button) const;

    /**
     * @brief Returns the mouse position.
     * @param x OUT the x position of the mouse.
     * @param y OUT the y position of the mouse.
     */
    void getMousePosition(int& x, int& y) const;
    /**
     * @brief Returns the mouse position from the last frame.
     * @param x OUT the x position of the mouse.
     * @param y OUT the y position of the mouse.
     */
    void getPreviousMousePosition(int& x, int& y) const;

public:
    virtual ~IInputSystem();
    explicit IInputSystem(const unsigned long derivedSize) {memorySize = derivedSize;}

    [[nodiscard]] unsigned long getMemorySize() const {return memorySize;}

    /**
     * @brief Called once each frame
     * @param deltaTime Time this frame took.
     */
    void update(double deltaTime);
    void initialize(EngineEvents* engineEventsRef);

    /**
     * @brief Called when the platform detects keyboard input
     * @param key The key that the user interacted with
     * @param bIsPressed Whether the key was pressed or released
     */
    void processKey(Keys key, bool bIsPressed);

    /**
     * @brief Called when the platform detects mouse input
     * @param button The button the user interacted with
     * @param bIsPressed Whether the button was pressed or released
     */
    void processButton(Buttons button, bool bIsPressed);

    /**
     * @brief Called when the platform detects mouse movement
     * @param x X pos of the mouse
     * @param y Y pos of the mouse
     */
    void processMouseMove(short x, short y);

    /**
     * @brief Called when the platform detects mouse scrolling
     * @param z Value of the mouse scroll
     */
    void processMouseScroll(char z) const;

    /**
     * @brief
     * @param key The key to check
     * @return True if the key is down.
     */
    [[nodiscard]] bool isKeyDown(Keys key) const;
    /**
     * @brief
     * @param key The key to check
     * @return True if the key is up.
     */
    [[nodiscard]] bool isKeyUp(Keys key) const;
    /**
     * @brief
     * @param key The key to check
     * @return True if the key was down on the last frame.
     */
    [[nodiscard]] bool wasKeyDown(Keys key) const;
    /**
     * @brief
     * @param key The key to check
     * @return True if the key was up on the last frame.
     */
    [[nodiscard]] bool wasKeyUp(Keys key) const;

    /**
     * @brief Subscribes a listener to an engine vent
     * @param code Engine event code to subscribe to
     * @param function Function to be called
     * @param id Unqiue Id of this listener
     * @param key Key to listen for (If Applicable)
     */
    void subscribeToEngineEvent(EngineEventCode code, const std::function<void(EngineInputContext)> &function, const String &id, Keys key = MAX_KEYS) const;
};