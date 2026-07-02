#pragma once

#include "FF_Memory.h"
#include "Platform.h"
#include "GameInstance.h"
#include "foxfire_export.h"
#include "IInputSystem.h"

class Engine {
private:
    //Handles the OS
    Platform* platform;
    //Handles advanced memory allocation
    FF_Memory* ff_memory;
    //Handle Input
    IInputSystem* inputSystem;

    GameInstance* gameInstance;
    bool bIsRunning = false;
    bool bIsPaused = false;
    bool bIsInitialized = false;
    short width;
    short height;
    float lastTime;

    void startup();
    void run();

protected:
    FOXFIRE_API void quit(EngineInputContext context);

public:
    explicit FOXFIRE_API Engine(GameInstance& instance, FF_Memory& mainMemory);
    FOXFIRE_API ~Engine();
};
