//
// Created by cmorg on 7/2/2026.
//

#pragma once
#include "IRenderSystem.h"
#include "ITextureSystem.h"
#include "RendererBackend.h"
#include "src/defines.h"
#include "src/modules/engine/Core/Platform.h"

class FOXFIRE_API MasterRenderSystem {
private:
    //Backend Renderer
    RendererBackend* backend = nullptr;
    //Camera projection
    Mat4 projection{};
    Mat4 view{};
    float nearClip = 0.1f;
    float farClip = 1000.0f;
    Texture testTexture{}; //remove me

    //Texture manager
    ITextureSystem* textureSystem = nullptr;

    DynamicArray<IRenderSystem> renderSystems{};

    [[nodiscard]] bool beginFrame(float deltaTime) const;
    [[nodiscard]] bool endFrame(float deltaTime) const;
    Texture createBlankTexture();

public:
    bool initialize(const String &appName, Platform& platform, const GameInstance& gameInstance, unsigned int width, unsigned int height);
    bool initializeTextureSystem(unsigned int initialCapacity, ITextureSystem* system);
    void shutdown();
    MasterRenderSystem() = default;
    ~MasterRenderSystem();

    RendererBackend* getBackend() const {return backend;}

    void setView(const Mat4 &newView);

    [[nodiscard]] bool drawFrame(const RenderPacket &packet);
    void onResize(unsigned short width, unsigned short height);
    void onDebugEvent();
    void createTexture(String name, int width, int height, int channelCount, const unsigned char *pixels, bool isTransparent, Texture &outTexture) const;
};
