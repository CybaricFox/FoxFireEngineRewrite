//
// Created by cmorg on 9/12/2026.
//

#include "UIRenderView.h"

#include <algorithm>

#include "src/modules/engine/ECS/EntityComponentSystem.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/ECS/Engine_Components/Transform.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Renderer/ShaderSystem.h"

bool UIRenderView::initialize(const unsigned long newSize) {
    IRenderView::initialize(newSize);
    const String shaderName = "Fox_Fire_UI_Shader";
    Resource configResource{};
    if (!ResourceSystem::load(shaderName, RESOURCE_TYPE_SHADER, configResource)) {
        Logger::logError("Failed to load ui shader.");
        return false;
    }

    ShaderConfig& config = *static_cast<ShaderConfig *>(configResource.data);
    if (!ShaderSystem::createShader(config, renderpasses[0])) {
        Logger::logError("Failed to create ui shader.");
        return false;
    }

    ResourceSystem::unload(configResource);

    shader = ShaderSystem::getShader(!customShaderName.isEmpty() ? customShaderName : shaderName);
    diffuseMapLocation = ShaderSystem::getUniformIndex(*shader, "diffuse_texture");
    diffuseColorLocation = ShaderSystem::getUniformIndex(*shader, "diffuse_color");
    modelLocation = ShaderSystem::getUniformIndex(*shader, "model");
    nearClip = -100;
    farClip = 100;
    projectionMatrix = orthographic(0, 1280, 720, 0, nearClip, farClip);
    viewMatrix = matrixIdentity();
    return true;
}

void UIRenderView::shutdown() {
    IRenderView::shutdown();
}

void UIRenderView::resize(const unsigned int newWidth, const unsigned int newHeight) {
    if (width != newWidth || height != newHeight) {
        width = newWidth;
        height = newHeight;
        projectionMatrix = orthographic(0, width, height, 0, nearClip, farClip);

        for (unsigned int i = 0; i < renderpassCount; i++) {
            renderpasses[i].setRenderArea({0, 0, static_cast<float>(width), static_cast<float>(height)});
        }
    }
}

bool UIRenderView::buildPacket(void *data, RenderViewPacket &outPacket) {
    if (!data) {
        Logger::logWarn("UI packet data is null! It cannot be built!");
        return false;
    }
    const UIPacketData& packetData = *static_cast<UIPacketData *>(data);

    outPacket.geometries.initialize();
    outPacket.renderView = this;
    outPacket.projectionMatrix = projectionMatrix;
    outPacket.viewMatrix = viewMatrix;
    outPacket.data = data;

    for (unsigned int i = 0; i < packetData.meshData.meshCount; i++) {
        Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(packetData.meshData.meshes[i]);
        for (unsigned int j = 0; j < mesh.geometryCount; j++) {
            Transform& transform = *EntityComponentSystem::getComponent<Transform>(packetData.meshData.meshes[i]);
            GeometryRenderData& renderData = *outPacket.geometries.emplace();
            renderData.geometry = mesh.geometries[j];
            renderData.model = TransformUtils::getWorldPos(transform);
            outPacket.geometryCount++;
        }
    }

    return true;
}

bool UIRenderView::render(RenderViewPacket &outPacket, const unsigned long frameNumber, const unsigned long renderTargetIndex, IRendererBackend *backendRef) {
    const unsigned int shaderId = shader->getId();

    for (unsigned int i = 0; i < renderpassCount; i++) {
        Renderpass& renderpass = renderpasses[i];
        if (!backendRef->beginRenderpass(renderpass, renderpass.getRenderTarget(renderTargetIndex))) {
            Logger::logError("Failed to begin ui render pass.");
            return false;
        }
        if (!ShaderSystem::use(shaderId)) {
            Logger::logError("Failed to use material shader.");
            return false;
        }
        if (!MaterialSystem::applyGlobal(shaderId, frameNumber, &outPacket.projectionMatrix, &outPacket.viewMatrix, nullptr, nullptr, 0)) {
            Logger::logError("Failed to apply globals for material shader.");
            return false;
        }

        const unsigned int count = outPacket.geometryCount;
        for (unsigned int i = 0; i < count; i++) {
            Material* material = nullptr;

            if (outPacket.geometries[i].geometry->material) {
                material = outPacket.geometries[i].geometry->material;
            } else {
                material = &MaterialSystem::getDefaultMaterial();
            }

            const bool needsUpdate = material->frameNumber != frameNumber;

            if (!MaterialSystem::applyInstance(*material, needsUpdate)) {
                Logger::logWarn("Failed to apply material " + material->name + " to ui apply instance.");
                continue;
            }

            material->frameNumber = frameNumber;

            MaterialSystem::applyLocal(*material, &outPacket.geometries[i].model);

            //In this case, the default material has the default texture.
            backendRef->drawGeometry(outPacket.geometries[i], MaterialSystem::getDefaultMaterial());
        }

        //Draw UI Text
        const UIPacketData& packetData = *static_cast<UIPacketData *>(outPacket.data);
        for (unsigned int j = 0; j < packetData.textCount; j++) {
            UIText& text = *packetData.texts[j];
            ShaderSystem::bindInstance(text.getInstanceId());

            if (!ShaderSystem::setUniform(diffuseMapLocation, &text.getTextureAtlas())) {
                Logger::logError("Failed to set uniform for bitmap font diffuse map.");
                return false;
            }

            //Font color
            static Vector4f white{1, 1, 1, 1};
            if (!ShaderSystem::setUniform(diffuseColorLocation, &white)) {
                Logger::logError("Failed to set uniform for bitmap font diffuse color.");
                return false;
            }

            const bool needsUpdate = text.getFrameNumber() != frameNumber;
            ShaderSystem::applyInstance(needsUpdate);
            text.setFrameNumber(frameNumber);

            //Apply locals
            Mat4 model = TransformUtils::getWorldPos(text.getTransform());
            if (!ShaderSystem::setUniform(modelLocation, &model)) {
                Logger::logError("Failed to set uniform for bitmap font model.");
            }

            text.draw();
        }

        if (!backendRef->endRenderpass(renderpass)) {
            Logger::logError("Failed to end ui render pass.");
            return false;
        }
    }

    return true;
}

void UIRenderView::destroyPacket(RenderViewPacket &packet) {
    packet.geometries.shutdown();
}
