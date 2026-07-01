#pragma once

#include "src/defines.h"

class Platform {
private:
    struct PlatformState {
        void* unknownState;
    };

public:
    Platform(const String &applicationName, int x, int y, int width, int height);
    ~Platform();

    static void printConsoleMessage(const char *message, unsigned char color);
    static void printConsoleError(const char *message, unsigned char color);
    bool processMessages();

    [[nodiscard]] float getAbsoluteTime() const;

private:
    PlatformState platformState;
};


