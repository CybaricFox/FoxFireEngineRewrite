//
// Created by cmorg on 7/1/2026.
//

#pragma once
#include "src/modules/engine/Input/IInputSystem.h"

class FoxFire_InputSystem final : public IInputSystem {
public:
    FoxFire_InputSystem();
    ~FoxFire_InputSystem() override = default;
};


/*
    What does this system need to do right now?

    A:
    Custom Event Creation
    There should be a registration function to register custom events
    Developer must define the callback and


*/