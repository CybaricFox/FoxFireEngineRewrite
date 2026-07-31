#pragma once

#include "../Library/Clock.h"
#include "Platform.h"
#include "GameInstance.h"
#include "foxfire_export.h"
#include "../Input/IInputSystem.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Memory/FF_Memory.h"
#include "src/modules/engine/Memory/LinearAllocator.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Renderer/MasterRenderSystem.h"
#include "src/modules/engine/Renderer/RendererBackend.h"

class FOXFIRE_API Engine {
private:
    //Block of memory that holds system memory data.
    MemoryBlock memoryDataStore{};
    LinearAllocator linearAllocator{};

    //Holds the log file
    FileHandler logHandler{};
    //Calculates system time
    Clock clock{};
    //Handles the OS
    Platform platform{};

    //Holds a pointer to the derived Game
    Engine* engine = nullptr;
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
    //Holds config data
    GameInstance* gameInstance = nullptr;
    //Handle Input
    IInputSystem* inputSystem = nullptr;
    //Texture manager reference (Owned by the masterrendersystem and should not be relied on)
    ITextureSystem* textureSystem = nullptr;
    //Frontend Rendering
    MasterRenderSystem masterRenderSystem{};

    void quit();
    virtual void startup();
    void run();
    void resize(unsigned short newWidth, unsigned short newHeight);
    virtual bool update (float deltaTime);
    bool render(float deltaTime);

    //REMOVE THIS LATER
    void setView(const Mat4 &newView);

public:
    explicit Engine();
    virtual ~Engine();

    void setEngineRef(Engine& derivedEngine);

    virtual void initialize(GameInstance& instance);

    void getFramebufferSize(unsigned int& bufferWidth, unsigned int& bufferHeight) const;
};
