/**
*   @file MasterRenderSystem.h
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
#include "IGeometrySystem.h"
#include "IMaterialSystem.h"
#include "ITextureSystem.h"
#include "IRendererBackend.h"
#include "src/defines.h"
#include "src/modules/engine/Core/Platform.h"

/**
 * @brief Controls and coordinates all rendering.
 */
class FOXFIRE_API MasterRenderSystem {
private:
    /** @brief pointer to the backend in use */
    IRendererBackend* backend = nullptr;
    Mat4 worldProjection{};
    Mat4 worldView{};
    Vector4f ambientColor{};
    Vector3f viewPosition{};
    Mat4 uiProjection{};
    Mat4 uiView{};
    /** @brief How close geometry can get before it is clipped. */
    float nearClip = 0.1f;
    /** @brief How far geometry can get before it is clipped. */
    float farClip = 1000.0f;

    /** @brief Pointer to the user defined texture system. */
    ITextureSystem* textureSystem = nullptr;
    /** @brief Pointer to the user defined material system. */
    IMaterialSystem* materialSystem = nullptr;
    /** @brief Pointer to the user defined Geometry system. */
    IGeometrySystem* geometrySystem = nullptr;

    ShaderSystem shaderSystem{};
    unsigned int materialShaderId = INVALID_ID_U32;
    unsigned int uiShaderId = INVALID_ID_U32;

    /** @brief Collection of renderpass profiles defined by the user. WARNING: Destroyed during initialization!*/
    DynamicArray<RenderpassProfile> renderpassProfiles{};
    /** @brief Whether this is initialized. */
    bool bIsInitialized = false;

    Texture createBlankTexture();
    void createRenderpasses();
    bool getRenderpassId(const String &name, unsigned char& outId);

    bool createShader(Shader& shader, unsigned char renderpassId, unsigned char stageCount, DynamicArray<String>& stageFileNames, DynamicArray<ShaderStage>& stages);
    void destroyShader(Shader &shader);
    bool initializeShader(Shader& shader);

public:
    bool initialize(const String &appName, Platform &platform, const GameInstance &gameInstance, unsigned int width, unsigned int height, ResourceSystem& resources);
    bool initializeTextureSystem(unsigned int initialCapacity, ITextureSystem *system, ResourceSystem *resourceSystem);
    bool initializeMaterialSystem(MaterialSystemConfig config, IMaterialSystem *system, ResourceSystem *resourceSystem);
    bool initializeGeometrySystem(unsigned int initialCapacity, IGeometrySystem *system, ResourceSystem *resourceSystem);
    bool initializeShaderSystem(const ShaderSystemConfig &config, ResourceSystem &resources);
    void shutdown();
    MasterRenderSystem() = default;

    [[nodiscard]] IRendererBackend* getBackend() const {return backend;}
    [[nodiscard]] Texture& getDefaultDiffuseTexture() const {return textureSystem->getDefaultDiffuseTexture();}
    [[nodiscard]] Texture& getDefaultSpecularTexture() const {return textureSystem->getDefaultSpecularTexture();}
    [[nodiscard]] Texture& getDefaultNormalTexture() const {return textureSystem->getDefaultNormalTexture();}
    [[nodiscard]] Geometry& getDefaultGeometry() const {return geometrySystem->getDefault3DGeometry();}

    void setView(const Mat4 &newView, Vector3f viewPosition);

    [[nodiscard]] bool drawFrame(const RenderPacket &packet);
    void onResize(unsigned short width, unsigned short height);
    [[nodiscard]] Texture& acquireTexture(bool autoRelease, const String &fileName, TextureUseCase useCase) const;
    void releaseTexture(const String &name) const;
    [[nodiscard]] Geometry& acquireGeometry(const GeometryConfig &config, bool autoRelease) const;
    void addRenderpassProfile(const RenderpassProfile &profile);

    [[nodiscard]] GeometryConfig generatePlaneConfig(float width, float height, unsigned int xCount, unsigned int yCount,
        float xTile, float yTile, const String &name, const String &materialName) const;
    [[nodiscard]] GeometryConfig generateCubeConfig(float width, float height, float depth, float xTile, float yTile, const String &name, const String &materialName) const;
};
