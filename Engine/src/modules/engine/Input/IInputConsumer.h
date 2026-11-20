//
// Created by cmorg on 11/20/2026.
//

#pragma once

#include "InputSystem.h"
#include "src/modules/engine/UserInterface/UIText.h"
#include "foxfire_export.h"

/**
 *  @file IInputConsumer.h
 *  @layer Engine
 *  @module Input
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/20/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API IInputConsumer {
protected:
    UIText entry{};

    virtual bool execute() = 0;
    virtual void exit() = 0;

public:
    virtual ~IInputConsumer() = default;

    virtual void onInput(Keys key, unsigned int modifiers);
};