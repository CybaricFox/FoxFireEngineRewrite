//
// Created by cmorg on 11/9/2026.
//

#include "PickRenderView.h"

#include "src/modules/engine/ECS/Engine_Components/Mesh.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/CameraUtils.h"
#include "src/modules/engine/ECS/Engine_ECS_Systems/TransformUtils.h"
#include "src/modules/engine/Library/UUID.h"
#include "src/modules/engine/Renderer/MasterRenderSystem.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

void PickRenderView::onMouseMoved(const EngineInputContext context) {
    mouseX = context.mouseX;
    mouseY = context.mouseY;
}

void PickRenderView::acquireShaderInstances() {
    unsigned int instance = 0;

    if (!MasterRenderSystem::acquireInstanceResources(*uiShaderData.shader, instance, nullptr)) {
        Logger::logFatal("Pick Render View failed to acquire shader resources for the ui.");
        return;
    }
    if (!MasterRenderSystem::acquireInstanceResources(*worldShaderData.shader, instance, nullptr)) {
        Logger::logFatal("Pick Render View failed to acquire shader resources for the world.");
        return;
    }

    instanceCount++;
    bIsInstanceUpdated.push(false);
}

void PickRenderView::releaseShaderInstances() {
    for (unsigned int i = 0; i < instanceCount; i++) {
        if (!MasterRenderSystem::releaseInstanceResources(*uiShaderData.shader, i)) {
            Logger::logWarn("Failed to release shader resources for the ui in pick view.");
        }
        if (!MasterRenderSystem::releaseInstanceResources(*worldShaderData.shader, i)) {
            Logger::logWarn("Failed to release shader resources for the world in pick view.");
        }
    }

    bIsInstanceUpdated.shutdown();
}

bool PickRenderView::initialize(const unsigned long newSize) {
    bIsInstanceUpdated.initialize(0);
    worldShaderData.renderpass = &renderpasses[0];
    uiShaderData.renderpass = &renderpasses[1];

    //UI Shader
    const String uiShaderName = "Fox_Fire_UI_Pick_Shader";
    Resource resource{};
    if (!ResourceSystem::load(uiShaderName, RESOURCE_TYPE_SHADER, resource)) {
        Logger::logError("Failed to load UI pick shader!");
        return false;
    }

    ShaderConfig& config = *static_cast<ShaderConfig *>(resource.data);
    if (!ShaderSystem::createShader(config, *uiShaderData.renderpass)) {
        Logger::logError("Failed to create UI pick shader!");
        return false;
    }

    ResourceSystem::unload(resource);

    uiShaderData.shader = ShaderSystem::getShader(uiShaderName);

    uiShaderData.idColorLocation = ShaderSystem::getUniformIndex(*uiShaderData.shader, "id_color");
    uiShaderData.modelLocation = ShaderSystem::getUniformIndex(*uiShaderData.shader, "model");
    uiShaderData.projectionLocation = ShaderSystem::getUniformIndex(*uiShaderData.shader, "projection");
    uiShaderData.viewLocation = ShaderSystem::getUniformIndex(*uiShaderData.shader, "view");

    uiShaderData.nearClip = -100;
    uiShaderData.farClip = 100;
    uiShaderData.fov = 0;
    uiShaderData.projection = orthographic(0, 1280, 720, 0, uiShaderData.nearClip, uiShaderData.farClip);
    uiShaderData.view = matrixIdentity();

    //World Shader
    const String worldShaderName = "Fox_Fire_World_Pick_Shader";
    Resource worldResource{};
    if (!ResourceSystem::load(worldShaderName, RESOURCE_TYPE_SHADER, worldResource)) {
        Logger::logError("Failed to load UI pick shader!");
        return false;
    }

    ShaderConfig& worldConfig = *static_cast<ShaderConfig *>(worldResource.data);
    if (!ShaderSystem::createShader(worldConfig, *worldShaderData.renderpass)) {
        Logger::logError("Failed to create UI pick shader!");
        return false;
    }

    ResourceSystem::unload(worldResource);

    worldShaderData.shader = ShaderSystem::getShader(worldShaderName);

    worldShaderData.idColorLocation = ShaderSystem::getUniformIndex(*worldShaderData.shader, "id_color");
    worldShaderData.modelLocation = ShaderSystem::getUniformIndex(*worldShaderData.shader, "model");
    worldShaderData.projectionLocation = ShaderSystem::getUniformIndex(*worldShaderData.shader, "projection");
    worldShaderData.viewLocation = ShaderSystem::getUniformIndex(*worldShaderData.shader, "view");

    worldShaderData.nearClip = 0.1f;
    worldShaderData.farClip = 1000;
    worldShaderData.fov = degreesToRadians(45);
    worldShaderData.projection = perspective(worldShaderData.fov, 1280 / 720.0f, worldShaderData.nearClip, worldShaderData.farClip);
    worldShaderData.view = matrixIdentity();

    //Register Events
    EngineEvents::subscribe(MOUSE_MOVED, [this](const EngineInputContext context){onMouseMoved(context);}, "Pick_Render_View");

    return IRenderView::initialize(newSize);
}

void PickRenderView::shutdown() {
    EngineEvents::unsubscribe(MOUSE_MOVED, "Pick_Render_View");

    releaseShaderInstances();
    MasterRenderSystem::destroyTexture(colorTargetAttachmentTexture);
    MasterRenderSystem::destroyTexture(depthTargetAttachmentTexture);

    bIsInstanceUpdated.shutdown();

    IRenderView::shutdown();
}

void PickRenderView::resize(const unsigned newWidth, const unsigned newHeight) {
    width = newWidth;
    height = newHeight;

    uiShaderData.projection = orthographic(0, width, height, 0, uiShaderData.nearClip, uiShaderData.farClip);

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    worldShaderData.projection = perspective(worldShaderData.fov, aspect, worldShaderData.nearClip, worldShaderData.farClip);

    for (unsigned int i = 0; i < renderpassCount; i++) {
        renderpasses[i].setRenderArea({0, 0, static_cast<float>(width), static_cast<float>(height)});
    }
}

bool PickRenderView::buildPacket(void *data, RenderViewPacket &outPacket) {
    if (!data) {
        Logger::logWarn("Pick Render View packet data cannot be null!");
        return false;
    }

    PickPacketData& packetData = *static_cast<PickPacketData *>(data);

    outPacket.geometries.initialize(0);
    outPacket.renderView = this;
    Camera& camera = *EntityComponentSystem::getComponent<Camera>(CameraSystem::getDefaultCamera());
    worldShaderData.view = CameraUtils::getViewMatrix(camera);
    outPacket.data = data;

    int highestInstanceId = 0;
    for (unsigned int i = 0; i < packetData.worldMeshData.meshCount; i++) {
        Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(packetData.worldMeshData.meshes[i]);
        for (unsigned int j = 0; j < mesh.geometryCount; j++) {
            GeometryRenderData renderData{};
            renderData.geometry = mesh.geometries[j];
            renderData.model = TransformUtils::getWorldPos(*EntityComponentSystem::getComponent<Transform>(packetData.worldMeshData.meshes[i]));
            renderData.id = mesh.id;
            outPacket.geometries.push(renderData);
            outPacket.geometryCount++;
            packetData.worldGeometryCount++;
        }

        if (mesh.id > highestInstanceId) {
            highestInstanceId = mesh.id;
        }
    }

    for (unsigned int i = 0; i < packetData.uiMeshData.meshCount; i++) {
        Mesh& mesh = *EntityComponentSystem::getComponent<Mesh>(packetData.uiMeshData.meshes[i]);
        for (unsigned int j = 0; j < mesh.geometryCount; j++) {
            GeometryRenderData renderData{};
            renderData.geometry = mesh.geometries[j];
            renderData.model = TransformUtils::getWorldPos(*EntityComponentSystem::getComponent<Transform>(packetData.uiMeshData.meshes[i]));
            renderData.id = mesh.id;
            outPacket.geometries.push(renderData);
            outPacket.geometryCount++;
            packetData.uiGeometryCount++;
        }

        if (mesh.id > highestInstanceId) {
            highestInstanceId = mesh.id;
        }
    }

    for (unsigned int i = 0; i < packetData.textCount; i++) {
        if (packetData.texts[i]->getId() > highestInstanceId) {
            highestInstanceId = packetData.texts[i]->getId();
        }
    }

    const unsigned int requiredInstanceCount = highestInstanceId + 1;

    if (requiredInstanceCount > instanceCount) {
        const unsigned int difference = requiredInstanceCount - instanceCount;
        for (unsigned int i = 0; i < difference; i++) {
            acquireShaderInstances();
        }
    }

    return true;
}

bool PickRenderView::render(RenderViewPacket &outPacket, unsigned long frameNumber, const unsigned long renderTargetIndex, IRendererBackend *backendRef) {
    unsigned int p = 0;
    Renderpass* renderpass = &renderpasses[p];

    if (renderTargetIndex == 0) {
        for (bool& value : bIsInstanceUpdated) {
            value = false;
        }

        if (!backendRef->beginRenderpass(*renderpass, renderpass->getRenderTarget(renderTargetIndex))) {
            Logger::logError("Pick Render View failed to start renderpass at target " + toString(renderTargetIndex));
            return false;
        }

        const PickPacketData& packetData = *static_cast<PickPacketData *>(outPacket.data);
        int currentInstanceId = 0;

        if (!ShaderSystem::use(worldShaderData.shader->getId())) {
            Logger::logError("Pick Render View failed to use shader " + worldShaderData.shader->getName());
            return false;
        }

        if (!ShaderSystem::setUniform(worldShaderData.projectionLocation, &worldShaderData.projection)) {
            Logger::logError("Failed to apply projection matrix to Pick Render View.");
        }
        if (!ShaderSystem::setUniform(worldShaderData.viewLocation, &worldShaderData.view)) {
            Logger::logError("Failed to apply view matrix to Pick Render View.");
        }

        ShaderSystem::applyGlobal();

        for (unsigned int i = 0; i < packetData.worldGeometryCount; i++) {
            GeometryRenderData& geometry = outPacket.geometries[i];
            currentInstanceId = geometry.id;
            ShaderSystem::bindInstance(currentInstanceId);

            Vector3f idColor{};
            unsigned int r, g, b = 0;
            uIntToRGB(geometry.id, r, g, b);
            idColor = rgbToVector3(r, g, b);

            if (!ShaderSystem::setUniform(worldShaderData.idColorLocation, &idColor)) {
                Logger::logError("Failed to apply id color uniform.");
                return false;
            }

            const bool needsUpdate = !bIsInstanceUpdated[currentInstanceId];
            ShaderSystem::applyInstance(needsUpdate);
            bIsInstanceUpdated[currentInstanceId] = true;

            if (!ShaderSystem::setUniform(worldShaderData.modelLocation, &geometry.model)) {
                Logger::logError("Failed to apply model uniform.");
            }

            //TODO: Remove default material requirement from this function!
            backendRef->drawGeometry(outPacket.geometries[i], MaterialSystem::getDefaultMaterial());
        }

        if (!backendRef->endRenderpass(*renderpass)) {
            Logger::logError("Failed to end world renderpass at index " + toString(p));
            return false;
        }

        p++;
        renderpass = &renderpasses[p];

        if (!backendRef->beginRenderpass(*renderpass, renderpass->getRenderTarget(renderTargetIndex))) {
            Logger::logError("Failed to begin renderpass at index " + toString(p));
            return false;
        }

        if (!ShaderSystem::use(uiShaderData.shader->getId())) {
            Logger::logError("Failed to use ui shader.");
            return false;
        }

        if (!ShaderSystem::setUniform(uiShaderData.projectionLocation, &uiShaderData.projection)) {
            Logger::logError("Failed to set projection matrix for the ui shader.");
        }
        if (!ShaderSystem::setUniform(uiShaderData.viewLocation, &uiShaderData.view)) {
            Logger::logError("Failed to set view matrix for the ui shader.");
        }

        ShaderSystem::applyGlobal();

        for (unsigned int i = packetData.worldGeometryCount; i < outPacket.geometryCount; i++) {
            GeometryRenderData& geometry = outPacket.geometries[i];
            currentInstanceId = geometry.id;

            ShaderSystem::bindInstance(currentInstanceId);

            Vector3f idColor{};
            unsigned int r, g, b = 0;
            uIntToRGB(geometry.id, r, g, b);
            idColor = rgbToVector3(r, g, b);

            if (!ShaderSystem::setUniform(uiShaderData.idColorLocation, &idColor)) {
                Logger::logError("Failed to apply id color uniform.");
                return false;
            }

            const bool needsUpdate = !bIsInstanceUpdated[currentInstanceId];
            ShaderSystem::applyInstance(needsUpdate);
            bIsInstanceUpdated[currentInstanceId] = true;

            if (!ShaderSystem::setUniform(uiShaderData.modelLocation, &geometry.model)) {
                Logger::logError("Failed to apply model uniform.");
            }

            //TODO: Remove default material requirement from this function!
            backendRef->drawGeometry(outPacket.geometries[i], MaterialSystem::getDefaultMaterial());
        }

        for (unsigned int i = 0; i < packetData.textCount; i++) {
            UIText& text = *packetData.texts[i];
            currentInstanceId = text.getId();
            ShaderSystem::bindInstance(currentInstanceId);

            Vector3f idColor{};
            unsigned int r, g, b = 0;
            uIntToRGB(text.getId(), r, g, b);
            idColor = rgbToVector3(r, g, b);

            if (!ShaderSystem::setUniform(uiShaderData.idColorLocation, &idColor)) {
                Logger::logError("Failed to apply id color uniform.");
                return false;
            }

            ShaderSystem::applyInstance(true);

            Mat4 model = TransformUtils::getWorldPos(text.getTransform());
            if (!ShaderSystem::setUniform(uiShaderData.modelLocation, &model)) {
                Logger::logError("Failed to apply model uniform.");
            }

            text.draw();
        }

        if (!backendRef->endRenderpass(*renderpass)) {
            Logger::logError("Failed to end ui renderpass at index " + toString(p));
            return false;
        }
    }

    unsigned char pixels[4]{};
    unsigned char* pixel = &pixels[0];

    unsigned short xCoord = std::clamp<short>(mouseX, 0, static_cast<short>(width) - 1);
    unsigned short yCoord = std::clamp<short>(mouseY, 0, static_cast<short>(width) - 1);

    backendRef->readPixel(colorTargetAttachmentTexture, xCoord, yCoord, &pixel);

    unsigned int id = rgbToUInt(pixel[0], pixel[1], pixel[2]);
    if (id == 0x00FFFFFF) {
        id = INVALID_ID_U32;
    }

    EngineEvents::callMouseHover(id);

    return true;
}

void PickRenderView::destroyPacket(RenderViewPacket &packet) {
    packet.geometries.shutdown();
}

bool PickRenderView::regenerateAttachmentTarget(const unsigned passIndex, RenderTargetAttachment &attachment) {
    if (attachment.type == RENDER_TARGET_ATTACHMENT_TYPE_COLOR) {
        attachment.texture = &colorTargetAttachmentTexture;
    } else if (attachment.type == RENDER_TARGET_ATTACHMENT_TYPE_DEPTH) {
        attachment.texture = &depthTargetAttachmentTexture;
    } else {
        Logger::logError("Invalid attachment type.");
        return false;
    }

    if (passIndex == 1) return true;

    if (attachment.texture->data) {
        MasterRenderSystem::destroyTexture(*attachment.texture);
    }

    const UUID textureUUID = UUIDUtils::generate();

    const unsigned int width = renderpasses[passIndex].getRenderArea().z;
    const unsigned int height = renderpasses[passIndex].getRenderArea().w;
    constexpr bool isTransparent = false;

    attachment.texture->id = INVALID_ID_U32;
    attachment.texture->type = TEXTURE_2D;
    attachment.texture->name = textureUUID.id;
    attachment.texture->width = width;
    attachment.texture->height = height;
    attachment.texture->channelCount = 4;
    attachment.texture->generation = INVALID_ID_U32;
    attachment.texture->flags |= isTransparent ? TEXTURE_BIT_TRANSPARENT : 0;
    attachment.texture->flags |= TEXTURE_BIT_WRITABLE;

    if (attachment.type == RENDER_TARGET_ATTACHMENT_TYPE_DEPTH) {
        attachment.texture->flags |= TEXTURE_BIT_DEPTH;
    }

    MasterRenderSystem::createWritableTexture(*attachment.texture);

    return true;
}
