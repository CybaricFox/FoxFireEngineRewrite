/**
 *  @file Engine.h
 *  @layer Engine
 *  @module Core
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "../Library/Clock.h"
#include "Platform.h"
#include "GameInstance.h"
#include "foxfire_export.h"
#include "Metrics.h"
#include "../Input/IInputSystem.h"
#include "src/modules/engine/ECS/EntityComponentSystem.h"
#include "src/modules/engine/Renderer/MasterRenderSystem.h"
#include "src/modules/engine/Threads/JobSystem.h"

#define MAX_THREAD_COUNT 15
#define AVERAGE_COUNT 30 //How many frames should pass before taking the average frames per ms?

/**
 * @brief The core of the engine.
 */
class FOXFIRE_API Engine {
private:
    /** @brief Handler for the log file */
    FileHandler logHandler{};
    /** @brief Calculates system time */
    Clock clock{};
    /** @brief Handles OS-specific variables */
    Platform platform{};
    /** @brief Handles resource loading and management */
    ResourceSystem resourceSystem{};
    /** @brief Handles user input handling. Input systems interface with this */
    EngineEvents engineEventsSystem{};
    /**
     * @brief Controls multithreading.
     */
    JobSystem jobSystem{};

    /** @brief Pointer to the derived game class set by the user. */
    Engine* engine = nullptr;

    /** @brief Whether the engine is running correctly. */
    bool bIsRunning = false;
    /** @brief Whether the engine is paused for some reason. */
    bool bIsPaused = false;
    /** @brief Whether the engine has finished initialization. */
    bool bIsInitialized = false;
    /** @brief The amount of time the previous frame took */
    double lastTime = 0;

    /**
     * @brief Initializes FF_Memory and the Linear Allocator
     */
    bool initializeMemory();

protected:
    //Protected Systems
    /** @brief Handles the Component system */
    EntityComponentSystem ECSSystem{};
    /** @brief Controls All Rendering.*/
    MasterRenderSystem masterRenderSystem{};

    //Holds config data
    /** @brief Holds Game-specific config data */
    GameInstance gameInstance{};
    /** @brief Pointer to the user-defined input system */
    IInputSystem* inputSystem = nullptr;

    LinearAllocator frameAllocator{};

    Metrics engineMetrics{};

    /**
     * @brief Quits the application when called.
     */
    void quit();

    /**
     * @brief Runs startup commands after all systems have been initialized.
     */
    virtual void startup();

    /**
     * @brief Starts and continues the run loop.
     */

    void run();
    /**
     * @brief Resizes the window
     * @param newWidth The new width of the window.
     * @param newHeight The new height of the window.
     */
    virtual void resize(unsigned short newWidth, unsigned short newHeight);

    /**
     * @brief Called once per frame.
     * @param deltaTime Time this frame took.
     * @return False if something went wrong.
     */
    virtual bool update (float deltaTime);

    /**
     * @brief Tells the render system to render. Called once per frame.
     * @param deltaTime Time this frame took.
     * @param renderPacket
     * @return False if something went wrong.
     */
    virtual bool render(float deltaTime, RenderPacket &renderPacket) = 0;

    bool bModelsLoaded = false;

    [[nodiscard]] unsigned int getWindowAttachmentCount() const;

    /**
     * @brief Sets up configs needed to initialize systems. Runs after memory initialization but before initialize();
     * @return
     */
    virtual bool boot() {
        if (engine == this) {
            Logger::logFatal("Engine set to itself! Engine should be set to a derived class!");
            return false;
        }

        return engine->boot();
    }

public:
    explicit Engine(const GameInstance& instance);
    virtual ~Engine();

    /**
     * @brief Sets the engine pointer. Do not call manually.
     * @param derivedEngine The user-defined game instance.
     */
    void setEngineRef(Engine& derivedEngine);

    /**
     * @brief Initializes systems.
     */
    virtual void initialize();
};
