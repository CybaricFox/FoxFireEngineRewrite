/**
*   @file Logger.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include <foxfire_export.h>

#include "FileHandler.h"
#include "src/defines.h"

//Enable or disable debug messages depending on the type of release
#define ENABLE_DEBUG_LOGGING 1

enum LogLevel {
    DEBUG = 4,
    INFO = 3,
    WARN = 2,
    SEVERE = 1,
    FATAL = 0
};

/**
 * @brief Logs messages to the platform
 */
class FOXFIRE_API Logger{
private:
    /** @brief Pointer to the log file */
    static FileHandler* logFile;

    /**
     * @brief Appends a message to the log file
     * @param message The message to be written
     */
    static void appendLog(const String& message);
public:
    /**
     * @brief Creates and overwrites the log file.
     * @param fileHandler Reference to the filehandler to handle the file.
     * @return True on success, False on failure.
     */
    static bool initializeFile(FileHandler& fileHandler);

    /**
     * @brief Closes the log file.
     */
    static void cleanup();

    /**
     * @brief Logs a message to the platform
     * @param level Level of the log. Controls which print function is called and what color the message will be.
     * @param message The message to log.
     */
    static void log(LogLevel level,const String &message);

    static void logFatal(const String &message) {log(FATAL, message);}
    static void logError(const String &message) {log(SEVERE, message);}
    static void logWarn(const String &message) {log(WARN, message);}
    static void logInfo(const String &message) {log(INFO, message);}
    static void logDebug(const String &message);
};