//
// Created by cmorg on 11/20/2026.
//

#include "FoxfireVar.h"

#include "StringUtils.h"

bool FoxfireVar::initialize() {
    ints.initialize(200);

    registerCommands();
    return true;
}

void FoxfireVar::shutdown() {
    ints.shutdown();
}

bool FoxfireVar::getInt(const String &name, int &outValue) {
    if (name.isEmpty()) return false;

    for (unsigned int i = 0; i < ints.getAssignedSize(); i++) {
        const FVInt* entry = ints.get(i);

        if (entry == nullptr) continue;

        if (entry->name.equalsIgnoreCase(name)) {
            outValue = entry->value;
            return true;
        }
    }

    Logger::logError("Failed to get an FVINT with the name " + name);
    return false;
}

bool FoxfireVar::setInt(const String &name, const int outValue) {
    if (name.isEmpty()) return false;

    for (unsigned int i = 0; i < ints.getAssignedSize(); i++) {
        FVInt* entry = ints.get(i);

        if (entry == nullptr) continue;

        if (entry->name.equalsIgnoreCase(name)) {
            entry->value = outValue;
            return true;
        }
    }

    Logger::logError("Failed to set an FVINT with the name " + name);
    return false;
}

void FoxfireVar::setInt(const ConsoleCommandContext context) {
    if (context.argumentCount != 2) {
        Logger::logError("FVINT set command requires 2 arguments.");
        return;
    }

    const String name = context.arguments[0].value;
    const String valueString = context.arguments[1].value;
    int value = 0;
    if (!StringUtils::stringToInt(valueString, value)) {
        Logger::logError("Failed to convert FVINT value argument to a number: " + valueString);
        return;
    }

    if (!setInt(name, value)) {
        Logger::logError("Failed to set FVINT with the name " + name);
        return;
    }

    MasterConsole::writeLine(INFO, name + " successfully set to " + toString(value));
}

bool FoxfireVar::createInt(const String &name, const int value) {
    if (name.isEmpty()) return false;

    const unsigned int index = ints.assign();

    if (index == FVINT_MAX_COUNT) {
        Logger::logError("FVINT reached maximum allowed FVINTs!");
        ints.release(index);
        return false;
    }

    FVInt& entry = *ints.get(index);
    entry.name = name;
    entry.value = value;

    return true;
}

void FoxfireVar::createInt(const ConsoleCommandContext context) {
    if (context.argumentCount != 2) {
        Logger::logError("FVINT create command requires 2 arguments.");
        return;
    }

    const String name = context.arguments[0].value;
    const String valueString = context.arguments[1].value;
    int value = 0;
    if (!StringUtils::stringToInt(valueString, value)) {
        Logger::logError("Failed to convert FVINT value argument to a number: " + valueString);
        return;
    }

    if (!createInt(name, value)) {
        Logger::logError("Failed to create FVINT with the name " + name);
        return;
    }

    MasterConsole::writeLine(INFO, name + " successfully created with initial value " + toString(value));
}

void FoxfireVar::printInt(const ConsoleCommandContext context) {
    if (context.argumentCount != 1) {
        Logger::logError("FVINT print command requires 1 argument.");
        return;
    }

    const String name = context.arguments[0].value;
    int value = 0;

    if (!getInt(name, value)) {
        Logger::logError("Failed to get FVINT with the name " + name);
        return;
    }

    MasterConsole::writeLine(INFO, "FVINT " + name + ": " + toString(value));
}

void FoxfireVar::registerCommands() {
    MasterConsole::registerCommand("create_int", 2, [this](const ConsoleCommandContext context){createInt(context);});
    MasterConsole::registerCommand("print_int", 1, [this](const ConsoleCommandContext context){printInt(context);});
    MasterConsole::registerCommand("set_int", 2, [this](const ConsoleCommandContext context){setInt(context);});
    MasterConsole::registerCommand("print_all_ints", 0, [this](const ConsoleCommandContext context){printAll();});
}

void FoxfireVar::printAll() {
    for (unsigned int i = 0; i < ints.getAssignedSize(); i++) {
        const FVInt* entry = ints.get(i);

        if (entry == nullptr) continue;

        MasterConsole::writeLine(INFO, "FVINT " + entry->name + ": " + toString(entry->value));
    }
}
