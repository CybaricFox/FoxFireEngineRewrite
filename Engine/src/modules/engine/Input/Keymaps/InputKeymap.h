//
// Created by cmorg on 11/20/2026.
//

#pragma once
#include "src/modules/engine/Input/IInputConsumer.h"
#include "src/modules/engine/Input/Keymap.h"

/**
 *  @file InputKeymap.h
 *  @layer 
 *  @module
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/20/2026
 *
 *  @copyright (c) 2026
 */

class InputKeymap {
private:
    static Keymap inputKeymap;
    static IInputConsumer* inputConsumer;

public:
    static void initialize();
    static void destroy();

    static void activate(IInputConsumer* consumer);
    static void deactivate();
};