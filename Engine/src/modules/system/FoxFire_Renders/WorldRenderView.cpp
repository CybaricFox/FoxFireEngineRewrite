//
// Created by cmorg on 9/12/2026.
//

#include "WorldRenderView.h"

#include "src/modules/engine/ECS/MasterEntityComponentSystem.h"
#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Renderer/MasterRenderSystem.h"

void WorldRenderView::onDebugEvent() {
    static char choice = 2;
    choice++;
    choice %= 3;

    switch (choice) {
        case 0: {
            Logger::logDebug("Render mode set to default.");
            renderMode = RENDER_VIEW_DEFAULT;
            break;
        }
        case 1: {
            Logger::logDebug("Render mode set to lighting.");
            renderMode = RENDER_VIEW_LIGHTING;
            break;
        }
        case 2: {
            Logger::logDebug("Render mode set to normals.");
            renderMode = RENDER_VIEW_NORMALS;
            break;
        }
        default: break;
    }
}

void WorldRenderView::quickSort(GeometryDistance* array, const int low, const int high, const bool ascending) {
    if (low < high) {
        const int partitionIndex = partition(array, low, high, ascending);

        quickSort(array, low, partitionIndex - 1, ascending);
        quickSort(array, partitionIndex + 1, high, ascending);
    }
}

void WorldRenderView::swapDistances(GeometryDistance& a, GeometryDistance& b) {
    const GeometryDistance temp = a;
    a = b;
    b = temp;
}

int WorldRenderView::partition(GeometryDistance* array, const int low, const int high, const bool ascending) {
    const GeometryDistance pivot = array[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (ascending) {
            if (array[j].distance < pivot.distance) {
                i++;
                swapDistances(array[i], array[j]);
            }
        } else {
            if (array[j].distance > pivot.distance) {
                i++;
                swapDistances(array[i], array[j]);
            }
        }
    }

    swapDistances(array[i + 1], array[high]);
    return i + 1;
}

bool WorldRenderView::initialize(ShaderSystem *shaderRef, const unsigned long newSize) {
    IRenderView::initialize(shaderRef, newSize);
    shaderId = shaderRef->getId(!customShaderName.empty() ? customShaderName : "Fox_Fire_Material_Shader");
    nearClip = 0.1f;
    farClip = 1000;
    fov = degreesToRadians(45.0f);
    projectionMatrix = perspective(fov, 1280.0f / 720.0f, nearClip, farClip);
    ambientColor = {0.25f, 0.25f, 0.25f, 1};
    return true;
}

void WorldRenderView::shutdown() {
    IRenderView::shutdown();
}

void WorldRenderView::resize(const unsigned int newWidth, const unsigned int newHeight) {
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

bool WorldRenderView::buildPacket(void *data, RenderViewPacket &outPacket) {
    if (!data) {
        Logger::logWarn("World packet data is null! It cannot be built!");
        return false;
    }
    const MeshPacketData& meshData = *static_cast<MeshPacketData *>(data);
    Camera& camera = *MasterEntityComponentSystem::getComponent<Camera>(worldCamera);

    outPacket.geometries.initialize();
    outPacket.renderView = this;
    outPacket.projectionMatrix = projectionMatrix;
    outPacket.viewMatrix = CameraUtils::getViewMatrix(camera);
    outPacket.viewPosition = CameraUtils::getPosition(camera);
    outPacket.ambientColor = ambientColor;

    DynamicArray<GeometryDistance> geometryDistances{0};


    for (unsigned int i = 0; i < meshData.meshCount; i++) {
        Mesh& mesh = *MasterEntityComponentSystem::getComponent<Mesh>(meshData.meshes[i]);
        Transform& transform = *MasterEntityComponentSystem::getComponent<Transform>(meshData.meshes[i]);
        Mat4 model = TransformUtils::getWorldPos(transform);

        for (unsigned int j = 0; j < mesh.geometryCount; j++) {
            GeometryRenderData renderData{};
            renderData.geometry = mesh.geometries[j];
            renderData.model = model;

            if ((mesh.geometries[j]->material->diffuseMap.texture->flags & TEXTURE_BIT_TRANSPARENT) == 0) {
                //Only add meshes that have no transparency
                outPacket.geometries.push(renderData);
                outPacket.geometryCount++;
            } else {
                const Vector3f center = transformVector3(renderData.geometry->center, model);
                float distance = getVectorDistance(center, camera.position);

                GeometryDistance& geometryDistance = *geometryDistances.emplace();
                geometryDistance.distance = FF_Math::abs(distance);
                geometryDistance.data = renderData;
            }
        }
    }

    quickSort(geometryDistances.getData(), 0, static_cast<int>(geometryDistances.getLength()) - 1, false);

    //Now add transparent geometries
    for (GeometryDistance& geometryDistance : geometryDistances) {
        outPacket.geometries.push(geometryDistance.data);
        outPacket.geometryCount++;
    }

    geometryDistances.shutdown();

    return true;
}

bool WorldRenderView::render(RenderViewPacket &outPacket, const unsigned long frameNumber, const unsigned long renderTargetIndex, IRendererBackend* backendRef, IMaterialSystem* materialSystemRef) {
    for (Renderpass* renderpass : renderpasses) {
        if (!backendRef->beginRenderpass(*renderpass, renderpass->getRenderTarget(renderTargetIndex))) {
            Logger::logError("Failed to begin ui render pass.");
            return false;
        }
        if (!shaderSystemRef->use(shaderId)) {
            Logger::logError("Failed to use material shader.");
            return false;
        }
        if (!materialSystemRef->applyGlobal(shaderId, frameNumber, &outPacket.projectionMatrix, &outPacket.viewMatrix, &outPacket.ambientColor, &outPacket.viewPosition, renderMode)) {
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

            bool needsUpdate = material->frameNumber != frameNumber;

            if (!materialSystemRef->applyInstance(*material, needsUpdate)) {
                Logger::logWarn("Failed to apply material " + material->name + " to ui apply instance.");
                continue;
            }

            material->frameNumber = frameNumber;

            materialSystemRef->applyLocal(*material, &outPacket.geometries[i].model);

            //In this case, the default material has the default texture.
            backendRef->drawGeometry(outPacket.geometries[i], *materialSystemRef->getDefaultMaterial().diffuseMap.texture, materialSystemRef->getDefaultMaterial());
        }

        if (!backendRef->endRenderpass(*renderpass)) {
            Logger::logError("Failed to end ui render pass.");
            return false;
        }
    }

    return true;
}
