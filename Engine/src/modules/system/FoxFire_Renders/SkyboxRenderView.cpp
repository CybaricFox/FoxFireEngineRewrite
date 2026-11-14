//
// Created by cmorg on 9/15/2026.
//

#include "SkyboxRenderView.h"

#include "src/modules/engine/ECS/EntityComponentSystem.h"
#include "src/modules/engine/ECS/Engine_Components/Camera.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraSystem.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"
#include "src/modules/engine/Renderer/MasterRenderSystem.h"
#include "src/modules/engine/Renderer/MaterialSystem.h"

void SkyboxRenderView::resize(const unsigned newWidth, const unsigned newHeight) {
    if (width != newWidth || height != newHeight) {

        width = newWidth;
        height = newHeight;
        const float aspect = static_cast<float>(width) / static_cast<float>(height);
        projectionMatrix = MatrixUtils::perspective(fov, aspect, nearClip, farClip);

        for (unsigned int i = 0; i < renderpassCount; i++) {
            renderpasses[i].setRenderArea({0, 0, static_cast<float>(width), static_cast<float>(height)});
        }
    }
}

bool SkyboxRenderView::buildPacket(void *data, RenderViewPacket &outPacket, LinearAllocator &allocator) {
    if (!data) {
        Logger::logWarn("Skybox packet data is null! It cannot be built!");
        return false;
    }
    Camera& camera = *EntityComponentSystem::getComponent<Camera>(worldCamera);

    outPacket.geometries.initialize();
    outPacket.renderView = this;
    outPacket.projectionMatrix = projectionMatrix;
    outPacket.viewMatrix = CameraUtils::getViewMatrix(camera);
    outPacket.viewPosition = CameraUtils::getPosition(camera);

    const SkyboxPacketData& packetData = *static_cast<SkyboxPacketData *>(data);
    outPacket.data = allocator.allocate(sizeof(SkyboxPacketData));
    FF_Memory::ff_copy(outPacket.data, &packetData, sizeof(SkyboxPacketData));

    return true;
}

bool SkyboxRenderView::render(RenderViewPacket &outPacket, const unsigned long frameNumber, const unsigned long renderTargetIndex) {
    const SkyboxPacketData& boxData = *static_cast<SkyboxPacketData *>(outPacket.data);
    const unsigned int shaderId = shader->getId();

    for (unsigned int i = 0; i < renderpassCount; i++) {
        Renderpass& renderpass = renderpasses[i];
        if (!MasterRenderSystem::beginRenderpass(renderpass, renderpass.getRenderTarget(renderTargetIndex))) {
            Logger::logError("Failed to begin skybox render pass.");
            return false;
        }
        if (!ShaderSystem::use(shaderId)) {
            Logger::logError("Failed to use skybox shader.");
            return false;
        }
        Camera& camera = *EntityComponentSystem::getComponent<Camera>(worldCamera);
        Mat4 view = CameraUtils::getViewMatrix(camera);
        view.data[12] = 0;
        view.data[13] = 0;
        view.data[14] = 0;
        MasterRenderSystem::bindShaderGlobals(*shader);
        if (!ShaderSystem::setUniform(projectionLocation, &projectionMatrix)) {
            Logger::logError("Failed to apply skybox projection.");
            return false;
        }
        if (!ShaderSystem::setUniform(viewLocation, &view)) {
            Logger::logError("Failed to apply skybox view.");
            return false;
        }
        ShaderSystem::applyGlobal();

        ShaderSystem::bindInstance(boxData.skybox->getInstanceId());

        if (!ShaderSystem::setUniform(cubeMapLocation, &boxData.skybox->getTextureMap())) {
            Logger::logError("Failed to apply skybox cube map.");
            return false;
        }
        const bool needsUpdate = boxData.skybox->getFrameNumber() != frameNumber;
        ShaderSystem::applyInstance(needsUpdate);
        GeometryRenderData renderData{};
        renderData.geometry = boxData.skybox->getGeometry();
        MasterRenderSystem::drawGeometry(renderData);
        if (!MasterRenderSystem::endRenderpass(renderpass)) {
            Logger::logError("Failed to end skybox render pass.");
            return false;
        }
    }
    return true;
}

bool SkyboxRenderView::initialize(const unsigned long newSize) {
    const String shaderName = "Fox_Fire_Skybox_Shader";
    Resource configResource{};
    if (!ResourceSystem::load(shaderName, RESOURCE_TYPE_SHADER, configResource)) {
        Logger::logError("Failed to load skybox shader.");
        return false;
    }

    ShaderConfig& config = *static_cast<ShaderConfig *>(configResource.data);
    if (!ShaderSystem::createShader(config, renderpasses[0])) {
        Logger::logError("Failed to create skybox shader.");
        return false;
    }

    ResourceSystem::unload(configResource);
    shader = ShaderSystem::getShader(!customShaderName.isEmpty() ? customShaderName : shaderName);
    projectionLocation = ShaderSystem::getUniformIndex(*shader, "projection");
    viewLocation = ShaderSystem::getUniformIndex(*shader, "view");
    cubeMapLocation = ShaderSystem::getUniformIndex(*shader, "cube_texture");
    nearClip = 0.1f;
    farClip = 1000.0f;
    fov = FF_Math::degreesToRadians(45.0f);

    projectionMatrix = MatrixUtils::perspective(fov, 1280 / 720.0f, nearClip, farClip);
    worldCamera = CameraSystem::getDefaultCamera();

    return IRenderView::initialize(newSize);
}

void SkyboxRenderView::shutdown() {
    IRenderView::shutdown();
}
