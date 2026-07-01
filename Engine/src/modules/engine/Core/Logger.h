#pragma once
#include <foxfire_export.h>
#include <string>

//Enable or disable debug messages depending on the type of release
#define ENABLE_DEBUG_LOGGING 1

enum LogLevel {
    DEBUG = 4,
    INFO = 3,
    WARN = 2,
    SEVERE = 1,
    FATAL = 0
};

class FOXFIRE_API Logger{
public:
    //Creates log file
    static void initializeFile();
    static void cleanup();

    //log a message to console
    static void log(LogLevel level,const std::string &message);

    static void logFatal(const std::string &message) {log(FATAL, message);}
    static void logError(const std::string &message) {log(SEVERE, message);}
    static void logWarn(const std::string &message) {log(WARN, message);}
    static void logInfo(const std::string &message) {log(INFO, message);}
    static void logDebug(const std::string &message);
};