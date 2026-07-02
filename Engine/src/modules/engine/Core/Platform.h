#pragma once

#include "EngineEvents.h"
#include "IInputSystem.h"
#include "src/defines.h"

class Platform {
private:
    struct PlatformState {
        void* unknownState;
    };

public:
    Platform();
    ~Platform();

    static void printConsoleMessage(const char *message, unsigned char color);
    static void printConsoleError(const char *message, unsigned char color);
    bool processMessages();

    [[nodiscard]] float getAbsoluteTime() const;

    void processInputs(IInputSystem& inputSystem);

    bool initialize(const String &applicationName, int x, int y, int width, int height);

private:
    PlatformState platformState;

};


