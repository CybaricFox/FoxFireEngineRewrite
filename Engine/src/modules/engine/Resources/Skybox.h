//
// Created by cmorg on 11/10/2026.
//

#pragma once
#include "foxfire_export.h"
#include "EngineTextureTypes.h"
#include "EngineTypes.h"

/**
 *  @file Skybox.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/10/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API Skybox {
private:
    TextureMap map{};
    Geometry* geometry = nullptr;
    unsigned int instanceId = INVALID_ID_U32;
    ULong frameNumber = INVALID_ID_U64;

public:
    bool initialize(const String &name);
    void shutdown();

    [[nodiscard]] unsigned int getInstanceId() const { return instanceId; }
    TextureMap& getTextureMap() { return map; }
    [[nodiscard]] ULong getFrameNumber() const { return frameNumber; }
    [[nodiscard]] Geometry* getGeometry() const { return geometry; }
};