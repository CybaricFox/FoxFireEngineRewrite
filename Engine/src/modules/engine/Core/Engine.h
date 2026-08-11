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
#include "../Input/IInputSystem.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Renderer/MasterRenderSystem.h"

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

    /** @brief Pointer to the derived game class set by the user. */
    Engine* engine = nullptr;

    /** @brief Whether the engine is running correctly. */
    bool bIsRunning = false;
    /** @brief Whether the engine is paused for some reason. */
    bool bIsPaused = false;
    /** @brief Whether the engine has finished initialization. */
    bool bIsInitialized = false;
    /** @brief Width of the screen. */
    unsigned short width = 0;
    /** @brief height of the screen. */
    unsigned short height = 0;
    /** @brief The amount of time the previous frame took */
    double lastTime = 0;

    //Remove Me
    Geometry* testGeometry = nullptr;
    Geometry* testUIGeometry = nullptr;

    /**
     * @brief Initializes FF_Memory and the Linear Allocator
     */
    bool initializeMemory();

protected:
    //Holds config data
    /** @brief Holds Game-specific config data */
    GameInstance gameInstance;
    /** @brief Pointer to the user-defined input system */
    IInputSystem* inputSystem = nullptr;
    /** @brief Controls All Rendering */
    MasterRenderSystem masterRenderSystem{};

    /** @brief Reference to the user-defined texture system. WARNING: VOLATILE REFERENCE! */
    ITextureSystem* textureSystem = nullptr;
    /** @brief Reference to the user-defined material system. WARNING: VOLATILE REFERENCE! */
    IMaterialSystem* materialSystem = nullptr;
    /** @brief Reference to the user-defined geometry system. WARNING: VOLATILE REFERENCE! */
    IGeometrySystem* geometrySystem = nullptr;

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
    void resize(unsigned short newWidth, unsigned short newHeight);

    /**
     * @brief Called once per frame.
     * @param deltaTime Time this frame took.
     * @return False if something went wrong.
     */
    virtual bool update (float deltaTime);

    /**
     * @brief Tells the render system to render. Called once per frame.
     * @param deltaTime Time this frame took.
     * @return False if something went wrong.
     */
    bool render(float deltaTime);

    void onDebugEvent() const {
        const String files[3] = {"whoishe", "Test1", "Test2"};
        const String specFiles[3] = {"", "Test1_SPEC", "Test2_SPEC"};
        const String normFiles[3] = {"", "Test1_NORM", ""};
        static char choice = 2;
        const String oldName = files[choice];
        const String oldSpecName = specFiles[choice];
        const String oldNormName = normFiles[choice];
        choice++;
        choice %= 3;

        if (testGeometry) {
            testGeometry->material->diffuseMap.texture = &masterRenderSystem.acquireTexture(true, files[choice], TEXTURE_USE_MAP_DIFFUSE);
            if (!testGeometry->material->diffuseMap.texture) {
                Logger::logWarn("Debug event failed to acquire texture!");
                testGeometry->material->diffuseMap.texture = &masterRenderSystem.getDefaultDiffuseTexture();
            }

            masterRenderSystem.releaseTexture(oldName);

            testGeometry->material->specularMap.texture = &masterRenderSystem.acquireTexture(true, specFiles[choice], TEXTURE_USE_MAP_SPECULAR);
            if (!testGeometry->material->specularMap.texture) {
                Logger::logWarn("Debug event failed to acquire texture!");
                testGeometry->material->specularMap.texture = &masterRenderSystem.getDefaultSpecularTexture();
            }

            masterRenderSystem.releaseTexture(oldSpecName);

            testGeometry->material->normalMap.texture = &masterRenderSystem.acquireTexture(true, normFiles[choice], TEXTURE_USE_MAP_NORMAL);
            if (!testGeometry->material->normalMap.texture) {
                Logger::logWarn("Debug event failed to acquire texture!");
                testGeometry->material->normalMap.texture = &masterRenderSystem.getDefaultNormalTexture();
            }

            masterRenderSystem.releaseTexture(oldNormName);
        }
    }

    /**
     * @brief Creates the derived GameState, Must be called after gameInstance is set by the Engine.
     * @tparam T Derived struct of BaseGameState set by the user.
     * @return Pointer to the constructed GameState.
     */
    template<typename T>
    requires std::derived_from<T, BaseGameState>
    T* createGameState() {
        T* derivedState = reinterpret_cast<T*>(gameInstance.state);
        std::construct_at(derivedState);
        return derivedState;
    }

    template<typename T>
    T* instantiateDerivedSubSystem() {
        return FF_Memory::ff_allocate_class<T>(sizeof(T), GAME);
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
     * @param instance Game config data.
     */
    virtual void initialize();

    /**
     * @brief Returns the window size.
     * @param bufferWidth OUT width of the window.
     * @param bufferHeight OUT height of the window.
     */
    void getFramebufferSize(unsigned int& bufferWidth, unsigned int& bufferHeight) const;
};
