#pragma once

#include "FF_Memory.h"
#include "Platform.h"
#include "IGame.h"
#include "foxfire_export.h"

class Game {
private:
    Platform* platform;
    FF_Memory* ff_memory;
    IGame* gameInstance;
    bool bIsRunning = false;
    bool bIsPaused = false;
    bool bIsInitialized = false;
    short width;
    short height;
    float lastTime;

    void startup();
    void run();

public:
    explicit FOXFIRE_API Game(IGame* instance, FF_Memory* mainMemory);
    FOXFIRE_API ~Game();
};
