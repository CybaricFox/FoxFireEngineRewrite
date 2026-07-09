//
// Created by cmorg on 6/30/2026.
//

#include "Logger.h"

#include "../Core/Platform.h"
#include "src/defines.h"

void Logger::initializeFile() {

}

void Logger::cleanup() {

}

void Logger::log(const LogLevel level, const String &message) {
    const String levelString[5] = {"[FATAL]: ", "[SEVERE]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: "};

    if (level < WARN) {
        Platform::printConsoleError(levelString[level] + message, level);
    } else {
        Platform::printConsoleMessage(levelString[level] + message, level);
    }
}

void Logger::logDebug(const String &message) {
    if constexpr (ENABLE_DEBUG_LOGGING == 1) {
        log(DEBUG, message);
    }
}
