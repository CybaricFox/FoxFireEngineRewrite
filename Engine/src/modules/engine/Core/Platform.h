#pragma once

#include "../Input/EngineEvents.h"
#include "../Input/IInputSystem.h"
#include "src/defines.h"

struct PlatformState {
    void* unknownState;
};

class Platform {
public:
    Platform();
    ~Platform();

    static void printConsoleMessage(const char *message, unsigned char color);
    static void printConsoleError(const char *message, unsigned char color);
    bool processMessages();

    [[nodiscard]] double getAbsoluteTime() const;

    void processInputs(IInputSystem& inputSystem);

    bool initialize(const String &applicationName, int x, int y, int width, int height);

    void ff_sleep(unsigned long ms);

    void getRequiredExtensions(std::vector<const char*>& extensions);

    PlatformState* getPlatformState() {return &platformState;}

    bool createSurface();

private:
    PlatformState platformState;

};


