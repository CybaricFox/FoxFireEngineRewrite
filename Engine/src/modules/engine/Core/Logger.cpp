//
// Created by cmorg on 6/30/2026.
//

#include "Logger.h"

#include <ostream>

#include "Platform.h"

void Logger::initializeFile() {

}

void Logger::cleanup() {

}

void Logger::log(const LogLevel level, const std::string &message) {
    const std::string levelString[5] = {"[FATAL]: ", "[SEVERE]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: "};

    if (level < WARN) {
        Platform::printConsoleError((levelString[level] + message).c_str(), level);
    } else {
        Platform::printConsoleMessage((levelString[level] + message).c_str(), level);
    }
}

void Logger::logDebug(const std::string &message) {
    if constexpr (ENABLE_DEBUG_LOGGING == 1) {
        log(DEBUG, message);
    }
}
