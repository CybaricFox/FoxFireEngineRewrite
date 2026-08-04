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
    //Handles system resources
    ResourceSystem resourceSystem{};

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

    //Remove Me
    Geometry* testGeometry = nullptr;
    Geometry* testUIGeometry = nullptr;

    void initializeMemory();

protected:
    //Holds config data
    GameInstance* gameInstance = nullptr;
    //Handle Input
    IInputSystem* inputSystem = nullptr;
    //Frontend Rendering
    MasterRenderSystem masterRenderSystem{};

    //Temp systems during startup. Not reliable!
    //Texture manager reference
    ITextureSystem* textureSystem = nullptr;
    //Material manager reference
    IMaterialSystem* materialSystem = nullptr;
    //Geometry manager reference
    IGeometrySystem* geometrySystem = nullptr;

    void quit();
    virtual void startup();
    void run();
    void resize(unsigned short newWidth, unsigned short newHeight);
    virtual bool update (float deltaTime);
    bool render(float deltaTime);

    //REMOVE THIS LATER
    void setView(const Mat4 &newView);

    void onDebugEvent() const {
        const String files[3] = {"whoishe", "Test1", "Test2"};
        static char choice = 2;
        const String oldName = files[choice];
        choice++;
        choice %= 3;

        if (testGeometry) {
            testGeometry->material->diffuseMap.texture = &masterRenderSystem.acquireTexture(true, files[choice]);
            if (!testGeometry->material->diffuseMap.texture) {
                Logger::logWarn("Debug event failed to acquire texture!");
                testGeometry->material->diffuseMap.texture = &masterRenderSystem.getDefaultTexture();
            }

            masterRenderSystem.releaseTexture(oldName);
        }
    }

public:
    explicit Engine();
    virtual ~Engine();

    void setEngineRef(Engine& derivedEngine);

    virtual void initialize(GameInstance& instance);

    void getFramebufferSize(unsigned int& bufferWidth, unsigned int& bufferHeight) const;
};
