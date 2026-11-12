//
// Created by cmorg on 11/10/2026.
//

#include "Skybox.h"

#include "src/modules/engine/Renderer/MasterRenderSystem.h"

bool Skybox::initialize(const String &name) {
    map.filterMag = TEXTURE_FILTER_BILINEAR;
    map.filterMin = TEXTURE_FILTER_BILINEAR;
    map.repeatU = TEXTURE_CLAMP_TO_EDGE;
    map.repeatV = TEXTURE_CLAMP_TO_EDGE;
    map.repeatW = TEXTURE_CLAMP_TO_EDGE;
    map.use = TEXTURE_USE_MAP_CUBE;

    if (!MasterRenderSystem::acquireTextureMapResources(map)) {
        Logger::logFatal("Failed to acquire resources for the skybox texture map.");
        return false;
    }

    map.texture = &TextureSystem::acquireCubeTexture("Maxwell_Skybox", true);
    GeometryConfig config = GeometrySystem::generateCubeConfig(10, 10, 10, 1, 1, name, {});
    config.materialName.destroy();
    geometry = &GeometrySystem::acquireGeometry(config, true);
    frameNumber = INVALID_ID_U64;
    const Shader& skyboxShader = *ShaderSystem::getShader("Fox_Fire_Skybox_Shader");
    TextureMap* maps[1]{&map};

    if (!MasterRenderSystem::acquireInstanceResources(skyboxShader, instanceId, maps)) {
        Logger::logFatal("Failed to acquire shader resources for the skybox texture map.");
        return false;
    }

    return true;
}

void Skybox::shutdown() {
    Shader* shader = ShaderSystem::getShader("Fox_Fire_Skybox_Shader");

    if (shader && instanceId != INVALID_ID_U32) {
        MasterRenderSystem::releaseInstanceResources(*shader, instanceId);
        instanceId = INVALID_ID_U32;
    }

    MasterRenderSystem::releaseTextureMapResources(map);
}
