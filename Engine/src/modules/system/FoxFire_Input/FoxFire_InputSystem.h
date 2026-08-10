/**
*   @file FoxFire_InputSystem.h
 *  @layer System
 *  @module FoxFire_Input
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "src/modules/engine/Input/IInputSystem.h"

/**
 * @brief Currently does nothing.
 */
class FOXFIRE_API FoxFire_InputSystem final : public IInputSystem {
public:
    FoxFire_InputSystem();
};


/*
    What does this system need to do right now?

    A:
    Custom Event Creation
    There should be a registration function to register custom events
    Developer must define the callback and


*/