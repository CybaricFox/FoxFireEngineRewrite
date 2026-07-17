//
// Created by cmorg on 6/30/2026.
//

#include "Logger.h"

#include <filesystem>

#include "FileHandler.h"
#include "../Core/Platform.h"
#include "src/defines.h"

FileHandler* Logger::logFile = nullptr;

void Logger::appendLog(const String &message) {
    const unsigned long length = message.length();
    unsigned long written = 0;
    if (!logFile->write(length, message.c_str(), written)) {
        logError("Failed to write log to file!");
    }
}

bool Logger::initializeFile(FileHandler& fileHandler) {
    logFile = &fileHandler;

    if (!logFile->exists("Logs")) {
        std::filesystem::create_directory("Logs");
    }

    if (!logFile->openFile("Logs/log.txt", WRITE, false)) {
        logError("Failed to open log file!");
        return false;
    }

    return true;
}

void Logger::cleanup() {
    logFile->closeFile();
    logFile = nullptr;
}

void Logger::log(const LogLevel level, const String &message) {
    const String levelString[5] = {"[FATAL]: ", "[SEVERE]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: "};

    if (level < WARN) {
        Platform::printConsoleError(levelString[level] + message, level);
    } else {
        Platform::printConsoleMessage(levelString[level] + message, level);
    }

    appendLog(levelString[level] + message + "\n");
}

void Logger::logDebug(const String &message) {
    if constexpr (ENABLE_DEBUG_LOGGING == 1) {
        log(DEBUG, message);
    }
}
