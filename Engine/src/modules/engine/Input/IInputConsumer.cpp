//
// Created by cmorg on 11/20/2026.
//

#include "IInputConsumer.h"

void IInputConsumer::onInput(const Keys key, const unsigned int modifiers) {
    const bool shift = (modifiers & KEYMAP_MODIFIER_SHIFT) != 0;

    switch (key) {
        case KEY_ESCAPE: {
            exit();
            return;
        }
        case KEY_ENTER: {
            if (entry.getSizeOfText() > 0) {
                if (!execute()) {
                    Logger::logError("Input consumer failed to execute input!");
                    return;
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
            int code = key;
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
                    case KEY_MINUS: code = shift ? '_' : '-'; break;
                    case KEY_PLUS: code = shift ? '+' : '='; break;
                    default: code = 0; break;
                }
            }

            const String stringCode = intToUTF8(code);

            if (code != 0) {
                const String newText = entry.getText();
                //If the first char is space, replace it instead of appending it.
                if (newText.getSizeInBytes() > 0 && newText.getAt(0) == ' ') {
                    entry.setText(stringCode);
                } else {
                    entry.setText(entry.getText() + stringCode);
                }
            }

            break;
        }
    }
}
