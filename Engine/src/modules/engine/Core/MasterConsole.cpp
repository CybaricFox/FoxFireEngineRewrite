//
// Created by cmorg on 11/17/2026.
//

#include "MasterConsole.h"

#include <cassert>

#include "src/modules/engine/Library/StringUtils.h"

MasterConsole* MasterConsole::instance = nullptr;

void MasterConsole::initialize() {
    instance = this;

    consoles = static_cast<IConsole **>(FF_Memory::ff_allocate_raw(sizeof(IConsole*) * MAX_CONSOLE_COUNT, ARRAY));
    commands.initialize(0);
}

void MasterConsole::shutdown() {
    for (unsigned int i = 0; i < MAX_CONSOLE_COUNT; i++) {
        if (consoles[i]) {
            consoles[i]->shutdown();
        }
    }

    FF_Memory::ff_free_raw(consoles, sizeof(IConsole*) * MAX_CONSOLE_COUNT, ARRAY);
    commands.shutdown();

    instance = nullptr;
}

void MasterConsole::registerConsole(IConsole *console) {
    assert(instance->consoleCount + 1 < MAX_CONSOLE_COUNT && "Max number of consoles exceeded");

    instance->consoles[instance->consoleCount] = console;
    instance->consoleCount++;
}

void MasterConsole::writeLine(const LogLevel level, const String &message) {
    //This can be called prior to initialization so check the instance state.
    if (!instance) return;

    for (unsigned int i = 0; i < instance->consoleCount; i++) {
        instance->consoles[i]->write(level, message);
    }
}

//This could be improved by using a hashmap
bool MasterConsole::registerCommand(const String &name, const unsigned char argumentCount, const std::function<void(ConsoleCommandContext)> &function) {
    if (name.isEmpty()) {
        Logger::logError("Command name cannot be empty!");
        return false;
    }

    for (unsigned int i = 0; i < instance->commands.getLength(); i++) {
        if (name == instance->commands[i].name) {
            Logger::logError("Command " + name + " already exists!");
            return false;
        }
    }

    ConsoleCommand& command = *instance->commands.emplace();
    command.argumentCount = argumentCount;
    command.function = function;
    command.name = name;

    return true;
}

bool MasterConsole::executeCommand(String commandLine) {
    if (commandLine.isEmpty()) return false;

    DynamicArray<String> args{};
    StringUtils::recursiveSplit(commandLine, ' ', args);

    if (args.getLength() < 1) {
        args.shutdown();
        return false;
    }

    instance->writeLine(INFO, commandLine);

    bool error = false;
    bool commandFound = false;

    for (unsigned int i = 0; i < instance->commands.getLength(); i++) {
        ConsoleCommand& command = instance->commands[i];
        if (command.name == args[0]) {
            commandFound = true;
            const unsigned char argumentCount = args.getLength() - 1;

            if (command.argumentCount != argumentCount) {
                Logger::logError("Console command " + command.name + " requires " + toString(command.argumentCount) + " arguments but only " + toString(argumentCount) + " were supplied.");
                error = true;
            } else {
                ConsoleCommandContext context{};
                context.argumentCount = command.argumentCount;
                if (context.argumentCount > 0) {
                    context.arguments = FF_Memory::ff_allocate_recursive<ConsoleCommandArgument>(ARRAY, context.argumentCount);
                    for (unsigned int j = 0; j < context.argumentCount; j++) {
                        context.arguments[j].value = args[j + 1];
                    }
                }

                command.function(context);

                if (context.arguments) {
                    FF_Memory::ff_free_recursive<ConsoleCommandArgument>(context.arguments, ARRAY, context.argumentCount);
                }
            }

            break;
        }
    }

    if (!commandFound) {
        Logger::logError("Command " + args[0] + " does not exist!");
        error = true;
    }

    args.shutdown();
    return !error;
}
