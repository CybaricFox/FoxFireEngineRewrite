#pragma once

#include "../Library/Clock.h"
#include "../Library/FF_Memory.h"
#include "Platform.h"
#include "GameInstance.h"
#include "foxfire_export.h"
#include "../Input/IInputSystem.h"
#include "src/modules/engine/Renderer/Renderer.h"
#include "src/modules/engine/Renderer/RendererBackend.h"

class FOXFIRE_API Engine {
private:
    //Handles advanced memory allocation
    FF_Memory ff_memory{};
    //Frontend Rendering
    Renderer renderer{};
    //Calculates system time
    Clock clock{};

    //Handles the OS
    Platform* platform;
    //Backend Renderer
    RendererBackend* backend;
    //Holds config data
    GameInstance* gameInstance;

    bool bIsRunning = false;
    bool bIsPaused = false;
    bool bIsInitialized = false;
    short width;
    short height;
    double lastTime;

protected:
    //Handle Input
    IInputSystem* inputSystem;

    void quit(EngineInputContext context);
    virtual void startup();
    void run();
    void resize(unsigned int width, unsigned int height);
    bool update (float deltaTime);
    bool render(float deltaTime);

public:
    Engine(GameInstance *instance, unsigned long stateSize);
    virtual ~Engine();

    void initialize(GameInstance& instance);
};
