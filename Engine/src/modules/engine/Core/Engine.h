#pragma once

#include "../Library/Clock.h"
#include "Platform.h"
#include "GameInstance.h"
#include "foxfire_export.h"
#include "../Input/IInputSystem.h"
#include "src/modules/engine/Renderer/Renderer.h"
#include "src/modules/engine/Renderer/RendererBackend.h"

class FOXFIRE_API Engine {
private:
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
    short width = 0;
    short height = 0;
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

    void getFramebufferSize(unsigned int* bufferWidth, unsigned int* bufferHeight) const;
};
