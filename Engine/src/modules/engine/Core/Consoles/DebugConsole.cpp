//
// Created by cmorg on 11/17/2026.
//

#include "DebugConsole.h"

#include "src/modules/engine/Core/MasterConsole.h"
#include "src/modules/engine/Input/InputSystem.h"
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

void DebugConsole::onInputPress(const EngineInputContext context) {
    if (!bIsVisible) return;

    const bool shift = InputSystem::isKeyDown(KEY_LSHIFT) || InputSystem::isKeyDown(KEY_RSHIFT) || InputSystem::isKeyDown(KEY_SHIFT);

    switch (context.key) {
        case KEY_ENTER: {
            if (entry.getSizeOfText() > 0) {
                if (!MasterConsole::executeCommand(entry.getText())) {

                }

                entry.setText("");
            }
            break;
        }
        case KEY_BACKSPACE: {
            if (entry.getSizeOfText() > 0) {
                String string = entry.getText();
                if (string.getSizeInBytes() <= 1) {
                    string = " ";
                } else {
                    string = string.substringLeft(string.getSizeInBytes() - 2);
                    string.trim();
                }

                entry.setText(string);
            }
            break;
        }
        case MAX_KEYS: {
            return;
        }
        default: {
            char code = context.key;
            if (code >= KEY_A && code <= KEY_Z) {
                if (!shift) code += 32;
            } else if (code >= KEY_0 && code <= KEY_9) {
                if (shift) {
                    switch (code) {
                        case KEY_0: code = ')'; break;
                        case KEY_1: code = '!'; break;
                        case KEY_2: code = '@'; break;
                        case KEY_3: code = '#'; break;
                        case KEY_4: code = '$'; break;
                        case KEY_5: code = '%'; break;
                        case KEY_6: code = '^'; break;
                        case KEY_7: code = '&'; break;
                        case KEY_8: code = '*'; break;
                        case KEY_9: code = '('; break;
                        default: break;
                    }
                }
            } else {
                switch (code) {
                    case KEY_SPACE: break;
                    default: code = 0; break;
                }
            }

            if (code != 0) {
                const String newText = entry.getText();
                //If the first char is space, replace it instead of appending it.
                if (newText.getSizeInBytes() > 0 && newText.getAt(0) == ' ') {
                    entry.setText(toString(code));
                } else {
                    entry.setText(entry.getText() + toString(code));
                }
            }

            break;
        }
    }
}

void DebugConsole::onInputRelease(EngineInputContext context) {
    //Do nothing for now
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

    EngineEvents::subscribe(KEY_PRESSED, [this](const EngineInputContext context) {onInputPress(context);}, "DebugConsole.InputPressed");
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
    lines.initialize(0);

    MasterConsole::registerConsole(this);
}

void DebugConsole::shutdown() {
    lines.shutdown();
    text.destroy();
    entry.destroy();
}
