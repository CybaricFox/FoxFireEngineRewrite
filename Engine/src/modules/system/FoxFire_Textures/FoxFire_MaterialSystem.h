//
// Created by cmorg on 8/1/2026.
//

#pragma once

#include "src/defines.h"
#include <foxfire_export.h>

#include "src/modules/engine/Library/AssetMap.h"
#include "src/modules/engine/Memory/HashMap.h"
#include "src/modules/engine/Renderer/IMaterialSystem.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Resources/Contexts.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

class FOXFIRE_API FoxFire_MaterialSystem final : public IMaterialSystem{
private:
    Material defaultMaterial{};
    AssetMap<Material, AssetContext> assets{};

    void shutdown();

    bool createDefaultMaterial();
    bool loadMaterial(const MaterialResourceData &config, Material& material) const;
    void destroyMaterial(Material& material) const;

public:
    ~FoxFire_MaterialSystem() override;

    bool initialize(unsigned int initialCapacity, ITextureSystem *system, RendererBackend *backend, ResourceSystem* resources) override;

    Material& acquireMaterial(const String &name) override;
    Material& acquireMaterial(const MaterialResourceData &config) override;
    void releaseMaterial(const String &name) override;

    Material& getDefaultMaterial() override {return defaultMaterial;}
};