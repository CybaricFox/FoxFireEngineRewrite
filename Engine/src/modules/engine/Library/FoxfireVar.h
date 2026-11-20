//
// Created by cmorg on 11/20/2026.
//

#pragma once
#include "ReusableArray.h"
#include "String.h"
#include "src/modules/engine/Core/IConsole.h"
#include "src/modules/engine/Core/MasterConsole.h"

/**
 *  @file FoxfireVar.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/20/2026
 *
 *  @copyright (c) 2026
 */

#define FVINT_MAX_COUNT 200

struct FVInt {
    String name{};
    int value = 0;
};

class FoxfireVar {
private:
    ReusableArray<FVInt> ints{};

    bool getInt(const String &name, int& outValue);

    bool setInt(const String &name, int outValue);
    void setInt(ConsoleCommandContext context);

    bool createInt(const String &name, int value);
    void createInt(ConsoleCommandContext context);
    void printInt(ConsoleCommandContext context);
    void registerCommands();
    void printAll();

public:
    bool initialize();
    void shutdown();
};