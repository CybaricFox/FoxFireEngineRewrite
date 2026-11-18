//
// Created by cmorg on 11/17/2026.
//

#pragma once
#include "src/modules/engine/Library/Logger.h"

class IConsole {
public:
    virtual ~IConsole() = default;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool write(LogLevel level, String message) = 0;
};
