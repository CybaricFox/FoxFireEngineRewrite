//
// Created by cmorg on 9/15/2026.
//

#include "SkyboxRenderView.h"

#include "src/modules/engine/ECS/MasterEntityComponentSystem.h"
#include "src/modules/engine/ECS/Engine_Components/Camera.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"

void SkyboxRenderView::resize(const unsigned newWidth, const unsigned newHeight) {
    if (width != newWidth || height != newHeight) {

        width = newWidth;
        height = newHeight;
        const float aspect = static_cast<float>(width) / static_cast<float>(height);
        projectionMatrix = perspective(fov, aspect, nearClip, farClip);

        for (Renderpass* renderpass : renderpasses) {
            renderpass->setRenderArea({0, 0, static_cast<float>(width), static_cast<float>(height)});
        }
    }
}

bool SkyboxRenderView::buildPacket(void *data, RenderViewPacket &outPacket) {
    if (!data) {
        Logger::logWarn("Skybox packet data is null! It cannot be built!");
        return false;
    }
    Camera& camera = *MasterEntityComponentSystem::getComponent<Camera>(worldCamera);

    outPacket.geometries.initialize();
    outPacket.renderView = this;
    outPacket.projectionMatrix = projectionMatrix;
    outPacket.viewMatrix = CameraUtils::getViewMatrix(camera);
    outPacket.viewPosition = CameraUtils::getPosition(camera);
    outPacket.data = data;

    return true;
}

bool SkyboxRenderView::render(RenderViewPacket &outPacket, const unsigned long frameNumber, const unsigned long renderTargetIndex, IRendererBackend *backendRef, IMaterialSystem *materialSystemRef) {
    const SkyboxPacketData& boxData = *static_cast<SkyboxPacketData *>(outPacket.data);

    for (Renderpass* renderpass : renderpasses) {
        if (!backendRef->beginRenderpass(*renderpass, renderpass->getRenderTarget(renderTargetIndex))) {
            Logger::logError("Failed to begin skybox render pass.");
            return false;
        }
        if (!shaderSystemRef->use(shaderId)) {
            Logger::logError("Failed to use skybox shader.");
            return false;
        }

        Camera& camera = *MasterEntityComponentSystem::getComponent<Camera>(worldCamera);
        Mat4 view = CameraUtils::getViewMatrix(camera);
        view.data[12] = 0;
        view.data[13] = 0;
        view.data[14] = 0;

        Shader& shader = *shaderSystemRef->getShader(shaderId);
        backendRef->bindShaderGlobals(shader);
        if (!shaderSystemRef->setUniform(projectionLocation, &projectionMatrix)) {
            Logger::logError("Failed to apply skybox projection.");
            return false;
        }
        if (!shaderSystemRef->setUniform(viewLocation, &view)) {
            Logger::logError("Failed to apply skybox view.");
            return false;
        }
        shaderSystemRef->applyGlobal();

        shaderSystemRef->bindInstance(boxData.skybox->instanceId);
        if (!shaderSystemRef->setUniform(cubeMapLocation, &boxData.skybox->map)) {
            Logger::logError("Failed to apply skybox cube map.");
            return false;
        }

        const bool needsUpdate = boxData.skybox->frameNumber != frameNumber;
        shaderSystemRef->applyInstance(needsUpdate);

        GeometryRenderData renderData{};
        renderData.geometry = boxData.skybox->geometry;
        backendRef->drawGeometry(renderData, *materialSystemRef->getDefaultMaterial().diffuseMap.texture,materialSystemRef->getDefaultMaterial());

        if (!backendRef->endRenderpass(*renderpass)) {
            Logger::logError("Failed to end skybox render pass.");
            return false;
        }
    }

    return true;
}

bool SkyboxRenderView::initialize(ShaderSystem *shaderRef, const unsigned long newSize) {
    Shader& shader = *shaderRef->getShader("Fox_Fire_Skybox_Shader");
    shaderId = shader.getId();
    projectionLocation = shader.getUniformIndex("projection");
    viewLocation = shader.getUniformIndex("view");
    cubeMapLocation = shader.getUniformIndex("cube_texture");

    nearClip = 0.1f;
    farClip = 1000.0f;
    fov = degreesToRadians(45.0f);

    projectionMatrix = perspective(fov, 1280 / 720.0f, nearClip, farClip);

    return IRenderView::initialize(shaderRef, newSize);
}

void SkyboxRenderView::shutdown() {
    IRenderView::shutdown();
}
