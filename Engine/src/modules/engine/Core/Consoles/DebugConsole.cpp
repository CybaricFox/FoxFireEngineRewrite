//
// Created by cmorg on 11/17/2026.
//

#include "DebugConsole.h"

#include "src/modules/engine/Core/MasterConsole.h"
#include "src/modules/engine/Input/InputSystem.h"
#include "src/modules/engine/Input/Keymaps/InputKeymap.h"
#include "src/modules/engine/Library/StringUtils.h"

bool DebugConsole::write(LogLevel level, String message) {
    DynamicArray<String> messages{};
    StringUtils::recursiveSplit(message, '\n', messages);

    for (String& string : messages) {
        lines.push(string);
    }

    bIsDirty = true;

    messages.shutdown();

    return true;
}

void DebugConsole::scrollUp(const unsigned int modifiers) {
    if ((modifiers & KEYMAP_MODIFIER_SHIFT) != 0) {
        return;
    }

    bIsDirty = true;

    if (lines.getLength() <= lineDisplayCount) {
        lineOffset = 0;
        return;
    }

    lineOffset++;
    lineOffset = FF_Math::min(lineOffset, static_cast<int>(lines.getLength() - lineDisplayCount));
}

void DebugConsole::scrollDown(const unsigned int modifiers) {
    if ((modifiers & KEYMAP_MODIFIER_SHIFT) != 0) {
        return;
    }

    bIsDirty = true;

    if (lines.getLength() <= lineDisplayCount) {
        lineOffset = 0;
        return;
    }

    lineOffset--;
    lineOffset = FF_Math::max(lineOffset, 0);
}

void DebugConsole::onInputRelease(EngineInputContext context) {
    //Do nothing for now
}

void DebugConsole::historyUp(const unsigned int modifiers) {
    const unsigned int length = commandHistory.getLength();
    if (length == 0) return;

    historyOffset = FF_Math::min(historyOffset + 1, static_cast<int>(length - 1));
    entry.setText(commandHistory[length - historyOffset - 1].command);
}

void DebugConsole::historyDown(const unsigned int modifiers) {
    const unsigned int length = commandHistory.getLength();
    if (length == 0 || historyOffset == -1) return;

    if (historyOffset == 0) {
        historyOffset = -1;
        entry.setText(" ");
        return;
    }

    historyOffset = FF_Math::max(historyOffset - 1, 0);
    entry.setText(commandHistory[length - historyOffset - 1].command);
}

bool DebugConsole::execute() {
    CommandHistoryEntry& commandEntry = *commandHistory.emplace();
    commandEntry.command = entry.getText();
    historyOffset = -1;
    if (!MasterConsole::executeCommand(entry.getText())) {
        //Handle failure in the future
    }

    return true;
}

void DebugConsole::exit() {
    InputSystem::pullKeymap();
    setVisibility(false);
}

void DebugConsole::setVisibility(const bool visible) {
    //Visibility must be changed
    if (bIsVisible == visible) return;

    bIsVisible = visible;

    if (visible) {
        InputKeymap::activate(this);
    }
}

bool DebugConsole::load() {
    if (!text.create(UI_TEXT_TYPE_SYSTEM, "Ubuntu-Regular", 31, " ")) {
        Logger::logFatal("Failed to create text for the debug console.");
        return false;
    }
    text.setPosition({3, 30, 0});

    if (!entry.create(UI_TEXT_TYPE_SYSTEM, "Ubuntu-Regular", 31, " ")) {
        Logger::logFatal("Failed to create entry for the debug console.");
        return false;
    }
    entry.setPosition({3, static_cast<float>(30 + (31 * lineDisplayCount)), 0});

    EngineEvents::subscribe(KEY_RELEASED, [this](const EngineInputContext context) {onInputRelease(context);}, "DebugConsole.InputReleased");

    return true;
}

void DebugConsole::update() {
    if (!bIsDirty) return;

    const unsigned int maxLines = FF_Math::min(lineDisplayCount, FF_Math::max(static_cast<int>(lines.getLength()), lineDisplayCount));
    const unsigned int minLine = FF_Math::max(static_cast<int>(lines.getLength() - maxLines - lineOffset), 0);
    const unsigned int maxLine = minLine + maxLines - 1;

    String buffer{};

    for (unsigned int i = minLine; i <= maxLine; i++) {
        buffer += lines[i] + "\n";
    }

    buffer += "\0";

    text.setText(buffer);
    bIsDirty = false;
}

void DebugConsole::initialize() {
    lineDisplayCount = 10;
    lines.initialize();
    commandHistory.initialize();

    MasterConsole::registerConsole(this);
}

void DebugConsole::shutdown() {
    lines.shutdown();
    commandHistory.shutdown();
    text.destroy();
    entry.destroy();
}
