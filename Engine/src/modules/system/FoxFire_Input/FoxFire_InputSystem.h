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
