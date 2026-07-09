#pragma once

#include "../Library/Clock.h"
#include "Platform.h"
#include "GameInstance.h"
#include "foxfire_export.h"
#include "../Input/IInputSystem.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Memory/LinearAllocator.h"
#include "src/modules/engine/Renderer/Renderer.h"
#include "src/modules/engine/Renderer/RendererBackend.h"

class FOXFIRE_API Engine {
private:
    //Block of memory that holds system memory data.
    MemoryBlock memoryDataStore{};
    LinearAllocator linearAllocator{};

    //Frontend Rendering
    Renderer renderer{};
    //Calculates system time
    Clock clock{};
    //Handles the OS
    Platform platform{};
    //Backend Renderer
    RendererBackend* backend = nullptr;

    //Holds a pointer to the derived Game
    Engine* engine = nullptr;
    //Holds config data
    GameInstance* gameInstance = nullptr;
    //Holds subscribers to engine related events
    DynamicArray<EngineEventCallback> subscribers[MAX_EVENT];

    bool bIsRunning = false;
    bool bIsPaused = false;
    bool bIsInitialized = false;
    short width = 0;
    short height = 0;
    double lastTime = 0;

    void initializeMemory();

protected:
    //Handle Input
    IInputSystem* inputSystem = nullptr;

    void quit();
    virtual void startup();
    void run();
    void resize(unsigned short newWidth, unsigned short newHeight);
    virtual bool update (float deltaTime);
    bool render(float deltaTime);

public:
    explicit Engine();
    virtual ~Engine();

    void setEngineRef(Engine& derivedEngine);

    void initialize(GameInstance& instance);

    void getFramebufferSize(unsigned int& bufferWidth, unsigned int& bufferHeight) const;
};
