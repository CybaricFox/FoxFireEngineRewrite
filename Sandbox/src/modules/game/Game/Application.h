#pragma once
#include "src/modules/engine/Core/Engine.h"
#include "../../../../../Engine/src/modules/engine/Input/EventSystem.h"
#include "src/modules/engine/Resources/Skybox.h"

struct ApplicationState : BaseApplicationState {
    float deltaTime = 0;
    unsigned int worldCamera = INVALID_ID_U32;
    unsigned short width = 0;
    unsigned short height = 0;
    Skybox skybox{};
    unsigned int worldEntities[10]{};
    unsigned int uiEntities[10]{};
    unsigned int maxwell = INVALID_ID_U32;
    bool bModelsLoaded = false;
    UIText bitmapText{};
    UIText systemText{};
    unsigned int hoveredId = 0;
    Frustum cameraFrustum{};
    DebugConsole debugConsole{};
    ULong allocationCount = 0;
    ULong previousAllocationCount = 0;
    Clock updateClock{};
    double lastUpdateTime = 0;
    Clock renderClock{};
    double lastRenderTime = 0;
};

struct ApplicationFrameData {
    DynamicArray<GeometryRenderData>* worldGeometries = nullptr;
};

class Application final : public Engine {
private:
    ApplicationFrameData frameData{};

    void loadModels();
    bool configureRenderViews();
public:
    explicit Application(const GameInstance& instance);
    ~Application() override;

    Event<void> swapTextureEvent{};
    Event<void> loadModelsEvent{};

protected:
    void startup() override;
    bool update(float deltaTime) override;
    void initialize() override;

    bool boot() override;

    bool render(float deltaTime, RenderPacket &renderPacket) override;
    void resize(unsigned short newWidth, unsigned short newHeight) override;
};


