//
// Created by cmorg on 11/17/2026.
//

#pragma once
#include <functional>

#include "IConsole.h"
#include "src/modules/engine/Library/Logger.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "foxfire_export.h"

/**
 *  @file MasterConsole.h
 *  @layer Engine
 *  @module Core
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/17/2026
 *
 *  @copyright (c) 2026
 */

#define MAX_CONSOLE_COUNT 10

struct ConsoleCommandArgument {
    String value{};
};

struct ConsoleCommandContext {
    unsigned char argumentCount = 0;
    ConsoleCommandArgument* arguments = nullptr;
};

struct ConsoleCommand {
    String name{};
    unsigned char argumentCount = 0;
    std::function<void(ConsoleCommandContext)> function = nullptr;
};

class FOXFIRE_API MasterConsole {
private:
    static MasterConsole* instance;
    unsigned char consoleCount = 0;
    IConsole** consoles = nullptr;
    DynamicArray<ConsoleCommand> commands{};

public:
    void initialize();
    void shutdown();

    static void registerConsole(IConsole* console);
    static void writeLine(LogLevel level, const String &message);
    static bool registerCommand(const String &name, unsigned char argumentCount, const std::function<void(ConsoleCommandContext)> &function);
    static bool executeCommand(String commandLine);
};