//
// Created by cmorg on 9/12/2026.
//

#include "RenderViewSystem.h"

#include "src/modules/system/FoxFire_Renders/PickRenderView.h"
#include "src/modules/system/FoxFire_Renders/SkyboxRenderView.h"
#include "src/modules/system/FoxFire_Renders/UIRenderView.h"
#include "src/modules/system/FoxFire_Renders/WorldRenderView.h"

bool RenderViewSystem::createRenderView(RenderViewConfig &config) {
    if (config.renderpasses.getLength() < 1) {
        Logger::logError("Render View " + config.name + " does not have any renderpasses.");
        return false;
    }
    if (config.name.isEmpty()) {
        Logger::logError("Render view requires a name.");
        return false;
    }

    if (assets.exists(config.name)) {
        Logger::logError("Attempted to create render view " + config.name + " but it already exists.");
        return false;
    }

    IRenderView** viewPtr = assets.createAsset(config.name, false);
    IRenderView* view = nullptr;

    if (!assets.exists(config.name)) {
        Logger::logError("Failed to create render view " + config.name + ".");
        return false;
    }

    unsigned long size = 0;
    switch (config.type) {
        case RENDER_VIEW_WORLD: {
            view = FF_Memory::ff_allocate<WorldRenderView>(RENDER);
            size = sizeof(WorldRenderView);
            break;
        }
        case RENDER_VIEW_UI: {
            view = FF_Memory::ff_allocate<UIRenderView>(RENDER);
            size = sizeof(UIRenderView);
            break;
        }
        case RENDER_VIEW_SKYBOX: {
            view = FF_Memory::ff_allocate<SkyboxRenderView>(RENDER);
            size = sizeof(SkyboxRenderView);
            break;
        }
        case RENDER_VIEW_PICK: {
            view = FF_Memory::ff_allocate<PickRenderView>(RENDER);
            size = sizeof(PickRenderView);
            break;
        }
    }
    *viewPtr = view;

    view->setId(assets.getAssetIndex(config.name));
    view->setType(config.type);
    view->setName(config.name);
    view->setCustomShader(config.customShaderName);
    view->setRenderpassCount(config.renderpasses.getLength());
    view->initializeRenderpasses();

    //Create renderpasses based on the config
    for (unsigned int i = 0; i < config.renderpasses.getLength(); i++) {
        if (!createRenderpass(config.renderpasses[i], view->getRenderpass(i))) {
            Logger::logError("Failed to create render pass " + config.renderpasses[i].name + ".");
            return false;
        }
    }

    if (!view->initialize(size)) {
        Logger::logError("Failed to creat render view.");
        view->shutdown();
        FF_Memory::ff_free<IRenderView>(view, RENDER, size);
        assets.releaseAsset(config.name);
        return false;
    }

    //Register Events
    EngineEvents::subscribe(REFRESH_RENDER_TARGETS, [this, view](EngineInputContext) {regenerateRenderTargets(*view);}, "Render_View_System_Refresh_" + view->getName());

    return true;
}

bool RenderViewSystem::createRenderpass(RenderpassConfig& config, Renderpass& outRenderpass) const {
    if (config.renderTargetCount == 0) {
        Logger::logError("A renderpass cannot be created with 0 render targets.");
        return false;
    }

    outRenderpass.setRenderTargetCount(config.renderTargetCount);
    outRenderpass.setTargets(FF_Memory::ff_allocate_recursive<RenderTarget>(ARRAY, config.renderTargetCount));
    outRenderpass.setClearFlags(config.clearFlags);
    outRenderpass.setClearColor(config.clearColor);
    outRenderpass.setRenderArea(config.renderArea);

    for (unsigned int i = 0; i < config.renderTargetCount; i++) {
        RenderTarget& target = outRenderpass.getRenderTarget(i);
        target.attachmentCount = config.targetConfig.attachments.getLength();
        target.attachments = FF_Memory::ff_allocate_recursive<RenderTargetAttachment>(ARRAY, config.targetConfig.attachments.getLength());

        for (unsigned int j = 0; j < config.targetConfig.attachments.getLength(); j++) {
            RenderTargetAttachment& attachment = target.attachments[j];
            const RenderTargetAttachmentConfig& attachmentConfig = config.targetConfig.attachments[j];

            attachment.source = attachmentConfig.source;
            attachment.type = attachmentConfig.type;
            attachment.loadOperation = attachmentConfig.loadOperation;
            attachment.storeOperation = attachmentConfig.storeOperation;
            attachment.texture = nullptr;
        }
    }

    return backendRef->createRenderpass(config, outRenderpass);
}

void RenderViewSystem::resize(const unsigned int width, const unsigned int height) {
    for (IRenderView** viewPtr : assets.getAssetsAsArray()) {
        IRenderView* view = *viewPtr;
        view->resize(width, height);
    }
}

bool RenderViewSystem::render(IRenderView &renderView, RenderViewPacket &packet, const unsigned long frameNumber, const unsigned long targetIndex) const {
    return renderView.render(packet, frameNumber, targetIndex);
}

void RenderViewSystem::regenerateRenderTargets(IRenderView &view) const {
    for (ULong i = 0; i < view.getRenderpassCount(); i++) {
        Renderpass& renderpass = view.getRenderpass(i);

        for (unsigned char j = 0; j < renderpass.getRenderTargetCount(); j++) {
            RenderTarget& target = renderpass.getRenderTarget(j);
            backendRef->destroyRenderTarget(target, false);

            for (unsigned int a = 0; a < target.attachmentCount; a++) {
                RenderTargetAttachment& attachment = target.attachments[a];

                if (attachment.source == RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT) {
                    if (attachment.type == RENDER_TARGET_ATTACHMENT_TYPE_COLOR) {
                        attachment.texture = backendRef->getWindowAttachment(j);
                    } else if (attachment.type == RENDER_TARGET_ATTACHMENT_TYPE_DEPTH) {
                        attachment.texture = backendRef->getDepthAttachment(j);
                    } else {
                        Logger::logFatal("Unsupported attachment type: " + toString(attachment.type));
                    }
                } else if (attachment.source == RENDER_TARGET_ATTACHMENT_SOURCE_VIEW) {
                    if (!view.regenerateAttachmentTarget(i, attachment)) {
                        Logger::logError("Failed to regenerate render view attachment: " + toString(attachment.type));
                    }
                }
            }

            backendRef->createRenderTarget(
                target.attachmentCount,
                target.attachments,
                renderpass,
                target.attachments[0].texture->width,
                target.attachments[0].texture->height,
                renderpass.getRenderTarget(j));
        }
    }
}

bool RenderViewSystem::initialize(const RenderViewSystemConfig config, IRendererBackend *backend) {
    backendRef = backend;

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

        for (unsigned int i = 0; i < view->getRenderpassCount(); i++) {
            Renderpass& renderpass = view->getRenderpass(i);

            for (unsigned int j = 0; j < renderpass.getRenderTargetCount(); j++) {
                backendRef->destroyRenderTarget(renderpass.getRenderTarget(j), true);
            }

            backendRef->destroyRenderpass(renderpass);
            renderpass.shutdown();
        }

        view->shutdown();
        FF_Memory::ff_free<IRenderView>(view,  RENDER, view->getSize());
    }

    assets.shutdown();

    UIText::shutdown();

    backendRef = nullptr;
}

IRenderView* RenderViewSystem::getRenderView(const String &name) {
    IRenderView** viewPtr = assets.getAsset(name);
    if (viewPtr == nullptr) return nullptr;
    return *viewPtr;
}
