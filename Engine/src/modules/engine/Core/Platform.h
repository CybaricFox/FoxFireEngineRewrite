#pragma once

#include "../Input/EngineEvents.h"
#include "../Input/IInputSystem.h"
#include "src/defines.h"

struct PlatformState {
    void* unknownState = nullptr;
};

class Platform {
public:
    Platform() = default;
    ~Platform();
    void shutdown();

    static void printConsoleMessage(const String& message, unsigned char color);
    static void printConsoleError(const String& message, unsigned char color);
    bool processMessages();

    static double getAbsoluteTime();

    void processInputs(IInputSystem& inputSystem);

    bool initialize(const String &applicationName, int x, int y, int width, int height);

    void ff_sleep(unsigned long ms);

    void getRequiredExtensions(DynamicArray<const char *> &extensions);

    PlatformState& getPlatformState() {return platformState;}

    bool createSurface();

private:
    PlatformState platformState{};

};


