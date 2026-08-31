/**
*   @file IRendererBackend.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "Shader.h"
#include "src/modules/engine/Core/GameInstance.h"
#include "src/modules/engine/Core/Platform.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

/**
 * @brief The Abstract Backend used for this application.
 */
class IRendererBackend {
private:
    /** @brief pointer to platform data */
    PlatformState* platformState = nullptr;
    /** @brief Current frame*/
    unsigned long frameNumber = 0;

protected:
    IRendererBackend() = default;
    ResourceSystem* resourceSystemRef = nullptr;

public:
    virtual ~IRendererBackend();

    /**
     * @brief Creates the specific backend.
     * @param type Type of backend to create.
     * @param newPlatformState Platform specific data.
     * @param gameInstance Game specific data.
     * @return The specific backend to use.
     */
    static IRendererBackend* create(RendererBackendType type, PlatformState& newPlatformState, const GameInstance& gameInstance);

    /**
     * @brief Returns the renderpass Id tied to this name
     * @param name Name of the renderpass
     * @param outId OUT Id of the renderpass
     * @return true if successful, false if renderpass does not exist
     */
    virtual bool getRenderpassId(String name, unsigned char& outId) = 0;

    /**
     * @brief Gets the current frame number.
     * @return The current frame number.
     */
    virtual unsigned int getFrameNumber() {return frameNumber;}

    /**
     * @brief Initializes the backend
     * @param appName Name of the application
     * @param platform A platform object reference (Will eventually be changed)
     * @param width Width of the window
     * @param height Height of the window
     * @param resources Pointer to the resource system for referencing.
     * @return False on failure
     */
    virtual bool initialize(String appName, Platform &platform, unsigned int width, unsigned int height, ResourceSystem* resources) = 0;

    /**
     * @brief Runs at the start of the frame
     * @param deltaTime time this frame took
     * @return false on failure
     */
    virtual bool beginFrame(float deltaTime) = 0;

    /**
     * @brief Runs at the end of the frame
     * @param deltaTime time this frame took
     * @return false on failure
     */
    virtual bool endFrame(float deltaTime) = 0;

    /**
     * @brief Resizes the window
     * @param width Width of the window
     * @param height Height of the window
     */
    virtual void resize(unsigned short width, unsigned short height) = 0;

    /**
     * @brief Draws geometry to screen
     * @param data Geometry data
     * @param defaultTexture Reference to the Texture Systems default texture
     * @param defaultMaterial Reference to the Material Systems default material
     */
    virtual void drawGeometry(const GeometryRenderData &data, Texture &defaultTexture, Material &defaultMaterial) = 0;

    /**
     * @brief Creates a texture out of pixel data
     * @param pixels Pixel data
     * @param texture OUT texture
     */
    virtual void createTexture(const unsigned char* pixels, Texture& texture) = 0;

    /**
     * @brief Destroys a texture
     * @param texture Texture to destroy
     */
    virtual void destroyTexture(Texture& texture) = 0;

    /**
     * @brief Creates geometry from vertex and index data
     * @param geometry OUT geometry
     * @param vertexSize Size of the vertex object
     * @param vertexCount Number of vertices
     * @param vertices Array of vertices
     * @param indexSize Size of the index object
     * @param indexCount Number of indices
     * @param indices Array of indices
     * @return False on failure
     */
    virtual bool createGeometry(Geometry &geometry, unsigned int vertexSize, unsigned int vertexCount, Vertex* vertices, unsigned int indexSize, unsigned int indexCount, void *indices) = 0;

    /**
     * @brief Destroys geometry
     * @param geometry Geometry to destroy
     */
    virtual void destroyGeometry(Geometry& geometry) = 0;

    /**
     * @brief Creates a renderpass from a renderpass profile
     * @param profile The profile the renderpass will use
     */
    virtual void createRenderpass(RenderpassProfile profile) = 0;

    /**
     * @brief Starts a renderpass
     * @param id id of the renderpass
     * @return false on failure
     */
    virtual bool beginRenderpass(unsigned char id) = 0;

    /**
     * @brief Ends a renderpass
     * @param id id of the renderpass
     * @return false on failure
     */
    virtual bool endRenderpass(unsigned char id) = 0;

    /**
     * @brief Creates a shader
     * @param shader OUT shader
     * @param renderpassId id of the renderpass this shader will use
     * @param stageCount Number of stages
     * @param stageFileNames
     * @param stages
     * @return false on failure
     */
    virtual bool createShader(Shader& shader, unsigned char renderpassId, unsigned char stageCount, DynamicArray<String>& stageFileNames, DynamicArray<ShaderStage>& stages) = 0;

    /**
     * @brief Finalizes a shader
     * @param shader Shader to finalize
     * @return false on failure
     */
    virtual bool initializeShader(Shader& shader) = 0;

    /**
     * @brief Destroys a shader
     * @param shader Shader to destroy
     */
    virtual void destroyShader(Shader& shader) = 0;

    /**
     * @brief Sets the shader to the current shader
     * @param shader Shader to use
     * @return false on failure
     */
    virtual bool useShader(Shader& shader) = 0;

    /**
     * @brief Binds shader global data
     * @param shader Shader to use
     * @return false on failure
     */
    virtual bool bindShaderGlobals(Shader& shader) = 0;

    /**
     * @brief Binds shader instance data
     * @param shader Shader to use
     * @param instanceId Instance to use
     */
    virtual void bindShaderInstance(Shader& shader, unsigned int instanceId) = 0;

    /**
     * @brief Sets a uniform within the shader
     * @param shader Shader to use
     * @param uniform Uniform to set
     * @param value Value that will be set
     * @return false on failure
     */
    virtual bool setUniform(Shader& shader, ShaderUniform& uniform, void* value) = 0;

    /**
     * @brief Applies shader globals to Global UBO
     * @param shader Shader to use
     * @return false on failure
     */
    virtual bool applyShaderGlobals(Shader& shader) = 0;

    /**
     * @brief Applies shader instance data to Instance
     * @param shader Shader to use
     * @param update Whether to update material data (Materials should only be updated once a frame)
     * @return false on failure
     */
    virtual bool applyShaderInstance(Shader& shader, bool update) = 0;

    /**
     * @brief
     * @param shader Shader to use
     * @param outInstanceId
     * @param defaultTexture Reference to the Texture Systems default texture
     * @return false on failure
     */
    virtual bool acquireInstanceResources(const Shader &shader, unsigned int &outInstanceId, Texture &defaultTexture) = 0;

    /**
     * @brief
     * @param shader Shader to use
     * @param instanceId
     * @return false on failure
     */
    virtual bool releaseInstanceResources(const Shader &shader, unsigned int instanceId) = 0;

    void incrementFrameNumber() {frameNumber++;}
    void clearFrameNumber() {frameNumber = 0;}
};