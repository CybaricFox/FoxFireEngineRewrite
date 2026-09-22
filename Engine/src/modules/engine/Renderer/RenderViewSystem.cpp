//
// Created by cmorg on 9/12/2026.
//

#include "RenderViewSystem.h"

#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraSystem.h"
#include "src/modules/system/FoxFire_Renders/SkyboxRenderView.h"
#include "src/modules/system/FoxFire_Renders/UIRenderView.h"
#include "src/modules/system/FoxFire_Renders/WorldRenderView.h"

bool RenderViewSystem::createRenderView(const RenderViewConfig &config) {
    if (config.renderpassCount < 1) {
        Logger::logError("Render View " + config.name + " does not have any renderpasses.");
        return false;
    }
    if (config.name.empty()) {
        Logger::logError("Render view requires a name.");
        return false;
    }

    const AssetContext context = assets.getContext(config.name);
    if (context.index != INVALID_ID_U32) {
        Logger::logError("Attempted to create render view " + config.name + " but it already exists.");
        return false;
    }

    AssetContext newContext{};
    IRenderView** viewPtr = assets.createAsset(config.name, newContext);
    IRenderView* view = nullptr;

    if (newContext.index == INVALID_ID_U32) {
        Logger::logError("Failed to create render view " + config.name + ".");
        return false;
    }

    unsigned long size = 0;
    switch (config.type) {
        case RENDER_VIEW_WORLD: {
            view = FF_Memory::ff_allocate_class<WorldRenderView>(sizeof(WorldRenderView), RENDER);
            size = sizeof(WorldRenderView);

            break;
        }
        case RENDER_VIEW_UI: {
            view = FF_Memory::ff_allocate_class<UIRenderView>(sizeof(UIRenderView), RENDER);
            size = sizeof(UIRenderView);
            break;
        }
        case RENDER_VIEW_SKYBOX: {
            view = FF_Memory::ff_allocate_class<SkyboxRenderView>(sizeof(SkyboxRenderView), RENDER);
            size = sizeof(SkyboxRenderView);
            break;
        }
    }
    *viewPtr = view;

    view->setId(newContext.index);
    view->setType(config.type);
    view->setName(config.name);
    view->setCustomShader(config.customShaderName);
    view->setRenderpassCount(config.renderpassCount);
    view->initializeRenderpasses();

    for (unsigned int i = 0; i < config.renderpassCount; i++) {
        Renderpass* renderpass = backendRef->getRenderpass(config.renderpasses[i].renderpassName);
        if (renderpass == nullptr) {
            Logger::logFatal("Failed to create render view because the renderpass: " + config.renderpasses->renderpassName + " could not be found.");
            return false;
        }
        view->addRenderpass(renderpass);
    }

    if (!view->initialize(shaderSystemRef, size)) {
        Logger::logError("Failed to creat render view.");
        view->shutdown();
        FF_Memory::ff_free_class<IRenderView>(view, size, RENDER);
        assets.releaseAsset(config.name);
        return false;
    }

    return true;
}

void RenderViewSystem::resize(const unsigned int width, const unsigned int height) {
    for (IRenderView** viewPtr : assets.getAssetsAsArray()) {
        IRenderView* view = *viewPtr;
        view->resize(width, height);
    }
}

bool RenderViewSystem::render(IRenderView &renderView, RenderViewPacket &packet, const unsigned long frameNumber, const unsigned long targetIndex) const {
    return renderView.render(packet, frameNumber, targetIndex, backendRef, materialSystemRef);
}

bool RenderViewSystem::initialize(const RenderViewSystemConfig config, IRendererBackend *backend, ShaderSystem* shaderSystem, IMaterialSystem* materialRef) {
    backendRef = backend;
    shaderSystemRef = shaderSystem;
    materialSystemRef = materialRef;

    if (config.maxViewCount == 0) {
        Logger::logFatal("Render view system config must have a max view count higher than 0!");
        return false;
    }

    maxViewCount = config.maxViewCount;
    assets.initialize(maxViewCount);

    return true;
}

void RenderViewSystem::shutdown() {
    for (IRenderView** viewPtr : assets.getAssetsAsArray()) {
        IRenderView* view = *viewPtr;
        view->shutdown();
        FF_Memory::ff_free_class<IRenderView>(view, view->getSize(), RENDER);
    }

    assets.shutdown();

    backendRef = nullptr;
    shaderSystemRef = nullptr;
    materialSystemRef = nullptr;
}

IRenderView* RenderViewSystem::getRenderView(const String &name) {
    IRenderView** viewPtr = assets.getAsset(name);
    if (viewPtr == nullptr) return nullptr;
    return *viewPtr;
}
