//
// Created by cmorg on 9/12/2026.
//

#include "UIRenderView.h"

#include "src/modules/engine/ECS/MasterEntityComponentSystem.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/ECS/Engine_Components/Transform.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Renderer/ShaderSystem.h"

bool UIRenderView::initialize(ShaderSystem* shaderSystem, const unsigned long newSize) {
    IRenderView::initialize(shaderSystem, newSize);
    shaderId = shaderSystemRef->getId(!customShaderName.empty() ? customShaderName : "Fox_Fire_UI_Shader");
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
            renderpasses[i]->setRenderArea({0, 0, static_cast<float>(width), static_cast<float>(height)});
        }
    }
}

bool UIRenderView::buildPacket(void *data, RenderViewPacket &outPacket) {
    if (!data) {
        Logger::logWarn("UI packet data is null! It cannot be built!");
        return false;
    }
    const MeshPacketData& meshData = *static_cast<MeshPacketData *>(data);

    outPacket.geometries.initialize();
    outPacket.renderView = this;
    outPacket.projectionMatrix = projectionMatrix;
    outPacket.viewMatrix = viewMatrix;

    for (unsigned int i = 0; i < meshData.meshCount; i++) {
        Mesh& mesh = *MasterEntityComponentSystem::getComponent<Mesh>(meshData.meshes[i]);
        for (unsigned int j = 0; j < mesh.geometryCount; j++) {
            Transform& transform = *MasterEntityComponentSystem::getComponent<Transform>(meshData.meshes[i]);
            GeometryRenderData& renderData = *outPacket.geometries.emplace();
            renderData.geometry = mesh.geometries[j];
            renderData.model = TransformUtils::getWorldPos(transform);
            outPacket.geometryCount++;
        }
    }

    return true;
}

bool UIRenderView::render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex, IRendererBackend *backendRef, IMaterialSystem* materialSystemRef) {
    for (Renderpass* renderpass : renderpasses) {
        if (!backendRef->beginRenderpass(*renderpass, renderpass->getRenderTarget(renderTargetIndex))) {
            Logger::logError("Failed to begin ui render pass.");
            return false;
        }
        if (!shaderSystemRef->use(shaderId)) {
            Logger::logError("Failed to use material shader.");
            return false;
        }
        if (!materialSystemRef->applyGlobal(shaderId, frameNumber, &outPacket.projectionMatrix, &outPacket.viewMatrix, nullptr, nullptr, 0)) {
            Logger::logError("Failed to apply globals for material shader.");
            return false;
        }

        const unsigned int count = outPacket.geometryCount;
        for (unsigned int i = 0; i < count; i++) {
            Material* material = nullptr;

            if (outPacket.geometries[i].geometry->material) {
                material = outPacket.geometries[i].geometry->material;
            } else {
                material = &materialSystemRef->getDefaultMaterial();
            }

            const bool needsUpdate = material->frameNumber != frameNumber;

            if (!materialSystemRef->applyInstance(*material, needsUpdate)) {
                Logger::logWarn("Failed to apply material " + material->name + " to ui apply instance.");
                continue;
            }

            material->frameNumber = frameNumber;

            materialSystemRef->applyLocal(*material, &outPacket.geometries[i].model);

            //In this case, the default material has the default texture.
            backendRef->drawGeometry(outPacket.geometries[i], *materialSystemRef->getDefaultMaterial().diffuseMap.texture, materialSystemRef->getDefaultMaterial());

            if (!backendRef->endRenderpass(*renderpass)) {
                Logger::logError("Failed to end ui render pass.");
                return false;
            }
        }
    }

    return true;
}
