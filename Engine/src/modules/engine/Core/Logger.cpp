//
// Created by cmorg on 6/30/2026.
//

#include "Logger.h"

#include "Platform.h"
#include "src/defines.h"

void Logger::initializeFile() {

}

void Logger::cleanup() {

}

void Logger::log(const LogLevel level, const String &message) {
    const String levelString[5] = {"[FATAL]: ", "[SEVERE]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: "};

    if (level < WARN) {
        Platform::printConsoleError((levelString[level] + message).c_str(), level);
    } else {
        Platform::printConsoleMessage((levelString[level] + message).c_str(), level);
    }
}

void Logger::logDebug(const String &message) {
    if constexpr (ENABLE_DEBUG_LOGGING == 1) {
        log(DEBUG, message);
    }
}
