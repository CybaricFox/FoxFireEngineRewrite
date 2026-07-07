#pragma once

#include "../Library/Clock.h"
#include "src/modules/engine/Memory/FF_Memory.h"
#include "Platform.h"
#include "GameInstance.h"
#include "foxfire_export.h"
#include "../Input/IInputSystem.h"
#include "src/modules/engine/Renderer/Renderer.h"
#include "src/modules/engine/Renderer/RendererBackend.h"

class FOXFIRE_API Engine {
private:
    //Holds a pointer to the derived Game
    Engine* engine = nullptr;
    //Block of memory that holds system memory data.
    MemoryBlock ff_memory;
    //Frontend Rendering
    Renderer renderer{};
    //Calculates system time
    Clock clock{};

    //Handles the OS
    Platform* platform;
    //Backend Renderer
    RendererBackend* backend = nullptr;
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

    void quit();
    virtual void startup();
    void run();
    void resize(unsigned short newWidth, unsigned short newHeight);
    virtual bool update (float deltaTime);
    bool render(float deltaTime);

public:
    Engine(GameInstance *instance, unsigned long stateSize);
    virtual ~Engine();

    void setEngineRef(Engine* derivedEngine);

    void initialize(GameInstance& instance);

    void getFramebufferSize(unsigned int* bufferWidth, unsigned int* bufferHeight) const;
};
