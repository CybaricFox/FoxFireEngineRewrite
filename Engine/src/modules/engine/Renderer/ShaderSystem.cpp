//
// Created by cmorg on 8/6/2026.
//

#include "ShaderSystem.h"

#include "TextureSystem.h"

ShaderSystem* ShaderSystem::instance = nullptr;

bool ShaderSystem::initialize(const ShaderSystemConfig newConfig, IRendererBackend *backend) {
    instance = this;

    backendRef = backend;

    if (newConfig.maxShaderCount < 512) {
        if (newConfig.maxShaderCount == 0) {
            Logger::logError("Shader system must have a max shader count greater than 0!");
            return false;
        }

        Logger::logWarn("Shader system should have a max shader count of 512 or greater.");
    }

    config = newConfig;
    assets.initialize(config.maxShaderCount);

    return true;
}

void ShaderSystem::shutdown() {
    for (Shader* shader : assets.getAssetsAsArray()) {
        destroyShader(*shader);
    }
    assets.shutdown();

    backendRef = nullptr;
    instance = nullptr;
}

unsigned int ShaderSystem::getId(const String &shaderName) {
    const unsigned int shaderId = instance->assets.getAssetIndex(shaderName);

    if (shaderId == INVALID_ID_U32) {
        Logger::logError("There is no registered shader named " + shaderName);
        return INVALID_ID_U32;
    }

    return shaderId;
}

Shader *ShaderSystem::getShader(const unsigned int shaderId) {
    if (shaderId >= instance->config.maxShaderCount || instance->assets.getAssetAtIndex(shaderId)->getId() == INVALID_ID_U32) {
        return nullptr;
    }

    return instance->assets.getAssetAtIndex(shaderId);
}

Shader *ShaderSystem::getShader(const String &shaderName) {
    const unsigned int shaderId = instance->getId(shaderName);
    if (shaderId != INVALID_ID_U32) {
        return getShader(shaderId);
    }

    return nullptr;
}

unsigned short ShaderSystem::getUniformIndex(Shader &shader, const String &uniformName) {
    if (shader.getId() == INVALID_ID_U32) {
        Logger::logError("Attepted to obtain uniform index from an invalid shader.");
        return INVALID_ID_U16;
    }

    unsigned int index = shader.getUniformIndex(uniformName);
    if (index == INVALID_ID_U32) {
        Logger::logError("Shader does not have a registered uniform named " + uniformName);
        return INVALID_ID_U16;
    }

    return index;
}

bool ShaderSystem::createShader(ShaderConfig &shaderConfig, Renderpass &renderpass) {
    Shader* shader = instance->assets.createAsset(shaderConfig.name, false);

    if (!shader->initializeShader(shaderConfig, instance->assets.getAssetIndex(shaderConfig.name))) return false;

    shader->setState(SHADER_STATE_NOT_INITIALIZED);

    for (unsigned int i = 0; i < shaderConfig.attributeCount; i++) {
        instance->addAttribute(*shader, shaderConfig.attributes[i]);
    }

    for (unsigned int i = 0; i < shaderConfig.uniformCount; i++) {
        if (shaderConfig.uniforms[i].type == SHADER_UNIFORM_TYPE_SAMPLER) {
            instance->addSampler(*shader, shaderConfig.uniforms[i]);
        } else {
            instance->addUniform(*shader, shaderConfig.uniforms[i]);
        }
    }

    int flags = 0;
    if (shaderConfig.bDepthTest) {
        flags |= SHADER_FLAG_DEPTH_TEST;
    }
    if (shaderConfig.bDepthWrite) {
        flags |= SHADER_FLAG_DEPTH_WRITE;
    }
    shader->setFlags(flags);


    if (!instance->backendRef->createShader(*shader, shaderConfig, renderpass, shaderConfig.stageCount, shaderConfig.stageFileNames, shaderConfig.stages)) {
        Logger::logError("Failed to create shader.");
        return false;
    }
    shader->ensureAttributeAlignment();

    if (!instance->backendRef->initializeShader(*shader)) {
        Logger::logError("Failed to initialize shader: " + shaderConfig.name);
        return false;
    }

    return true;
}

bool ShaderSystem::use(const String &name) {
    const unsigned int nextShaderId = getId(name);
    if (nextShaderId == INVALID_ID_U32) return false;

    return use(nextShaderId);
}

bool ShaderSystem::use(const unsigned int shaderId) {
    if (instance->currentShaderId != shaderId) {
        Shader* nextShader = getShader(shaderId);
        if (nextShader == nullptr) return false;
        instance->currentShaderId = shaderId;
        if (!instance->backendRef->useShader(*nextShader)) {
            Logger::logError("Failed to use shader " + nextShader->getName());
            return false;
        }
        if (!instance->backendRef->bindShaderGlobals(*nextShader)) {
            Logger::logError("Failed to bind globals for shader " + nextShader->getName());
            return false;
        }
    }

    return true;
}

bool ShaderSystem::setUniform(const String &uniformName, void *value) {
    if (currentShaderId == INVALID_ID_U32) {
        Logger::logError("Cannot set a uniform when no shader is in use!");
        return false;
    }

    Shader& shader = *assets.getAssetAtIndex(currentShaderId);
    return setUniform(getUniformIndex(shader, uniformName), value);
}

bool ShaderSystem::setUniform(const unsigned short index, void *value) {
    Shader& shader = *instance->assets.getAssetAtIndex(instance->currentShaderId);
    ShaderUniform& uniform = shader.getUniform(index);
    if (shader.getBoundScope() != uniform.scope) {
        switch (uniform.scope) {
            case SHADER_SCOPE_GLOBAL: {
                instance->backendRef->bindShaderGlobals(shader);
                break;
            }
            case SHADER_SCOPE_INSTANCE: {
                instance->backendRef->bindShaderInstance(shader, shader.getBoundInstanceId());
                break;
            }
            default: break;
        }
    }

    return instance->backendRef->setUniform(shader, uniform, value);
}

bool ShaderSystem::setSampler(const String &samplerName, Texture &texture) {
    return setUniform(samplerName, &texture);
}

bool ShaderSystem::setSampler(const unsigned short index, Texture &texture) {
    return setUniform(index, &texture);
}

bool ShaderSystem::applyGlobal() {
    return instance->backendRef->applyShaderGlobals(*instance->assets.getAssetAtIndex(instance->currentShaderId));
}

bool ShaderSystem::applyInstance(const bool update) {
    return instance->backendRef->applyShaderInstance(*instance->assets.getAssetAtIndex(instance->currentShaderId), update);
}

bool ShaderSystem::bindInstance(const unsigned int instanceId) {
    Shader& shader = *instance->assets.getAssetAtIndex(instance->currentShaderId);
    shader.setBoundInstanceId(instanceId);
    instance->backendRef->bindShaderInstance(shader, instanceId);

    return true;
}

bool ShaderSystem::addAttribute(Shader &shader, const ShaderAttributeConfig &attributeConfig) {
    unsigned int size = 0;

    switch (attributeConfig.type) {
        case SHADER_ATTRIBUTE_TYPE_INT8:
        case SHADER_ATTRIBUTE_TYPE_UINT8: {
            size = 1;
            break;
        }
        case SHADER_ATTRIBUTE_TYPE_INT16:
        case SHADER_ATTRIBUTE_TYPE_UINT16: {
            size = 2;
            break;
        }
        case SHADER_ATTRIBUTE_TYPE_FLOAT32:
        case SHADER_ATTRIBUTE_TYPE_INT32:
        case SHADER_ATTRIBUTE_TYPE_UINT32: {
            size = 4;
            break;
        }
        case SHADER_ATTRIBUTE_TYPE_FLOAT32_2: {
            size = 8;
            break;
        }
        case SHADER_ATTRIBUTE_TYPE_FLOAT32_3: {
            size = 12;
            break;
        }
        case SHADER_ATTRIBUTE_TYPE_FLOAT32_4: {
            size = 16;
            break;
        }
        default: {
            Logger::logError("Unrecognized attribute type: " + toString(attributeConfig.type));
            size = 4;
            break;
        }
    }

    shader.increaseAttributeStride(size);

    ShaderAttribute attribute{};
    attribute.name = attributeConfig.name;
    attribute.size = size;
    attribute.type = attributeConfig.type;
    shader.addAttribute(attribute);

    return true;
}

bool ShaderSystem::addSampler(Shader &shader, const ShaderUniformConfig &uniformConfig) {
    if (uniformConfig.scope == SHADER_SCOPE_LOCAL) {
        Logger::logError("Samplers cannot be used within local scope.");
        return false;
    }

    if (!isUniformNameValid(shader, uniformConfig.name) || !isUniformStateValid(shader)) {
        return false;
    }

    unsigned int location = 0;
    if (uniformConfig.scope == SHADER_SCOPE_GLOBAL) {
        unsigned int globalTextureCount = shader.getGlobalTextureCount();
        if (globalTextureCount + 1 > config.maxGlobalTextures) {
            Logger::logError("Shader global texture count exceeds " + toString(config.maxGlobalTextures));
            return false;
        }
        location = globalTextureCount;

        //Create default texture map
        TextureMap defaultMap{};
        defaultMap.filterMag = TEXTURE_FILTER_BILINEAR;
        defaultMap.filterMin = TEXTURE_FILTER_BILINEAR;
        defaultMap.repeatU = TEXTURE_REPEAT;
        defaultMap.repeatV = TEXTURE_REPEAT;
        defaultMap.repeatW = TEXTURE_REPEAT;
        defaultMap.use = TEXTURE_USE_UNKNOWN;
        if (!backendRef->acquireTextureMapResources(defaultMap)) {
            Logger::logError("Failed to acquire global texture map resources during shader creation.");
            return false;
        }
        const auto map = FF_Memory::ff_allocate<TextureMap>(RENDER);
        *map = defaultMap;
        map->texture = &TextureSystem::getDefaultDiffuseTexture();
        shader.addGlobalTextureMap(map);

    } else {
        if (shader.getInstanceTextureCount() + 1 > config.maxInstanceTextures) {
           Logger::logError("Shader instance texture count exceeds " + toString(config.maxInstanceTextures));
            return false;
        }
        location = shader.getInstanceTextureCount();
        shader.incrementInstanceTextureCount();
    }

    if (!addUniform(shader, uniformConfig.name, 0, uniformConfig.type, uniformConfig.scope, location, true)) {
        Logger::logError("Failed to add sampler " + uniformConfig.name);
        return false;
    }

    return true;
}

bool ShaderSystem::addUniform(Shader &shader, const ShaderUniformConfig &uniformConfig) {
    if (!isUniformStateValid(shader) || !isUniformNameValid(shader, uniformConfig.name)) {
        return false;
    }

    return addUniform(shader, uniformConfig.name, uniformConfig.size, uniformConfig.type, uniformConfig.scope, 0, false);
}

bool ShaderSystem::addUniform(Shader &shader, const String &uniformName, const unsigned int size, const ShaderUniformType type, const ShaderScope scope, const unsigned int descriptorLocation, const bool isSampler) const {
    if (shader.getUniformCount() + 1 > config.maxUniformCount) {
        Logger::logError("Number of shader uniforms and samplers cannot exceed " + toString(shader.getUniformCount()));
        return false;
    }

    ShaderUniform* uniform = shader.createUniform(uniformName, false);
    if (uniform == nullptr) {
        Logger::logError("Failed to create shader uniform " + uniformName);
        return false;
    }

    uniform->index = shader.getUniformIndex(uniformName);
    uniform->scope = scope;
    uniform->type = type;
    const bool isGlobal = scope == SHADER_SCOPE_GLOBAL;

    if (isSampler) {
        uniform->location = descriptorLocation;
    } else {
        uniform->location = uniform->index;
    }

    if (scope != SHADER_SCOPE_LOCAL) {
        uniform->descriptorIndex = scope;
        uniform->offset = isSampler ? 0 : isGlobal ? shader.getGlobalSize() : shader.getInstanceSize();
        uniform->size = isSampler ? 0 : size;
    } else {
        uniform->descriptorIndex = INVALID_ID_U8;
        const MemoryRange range = getAlignedRange(shader.getPushConstantSize(), size, 4);
        uniform->offset = range.offset;
        uniform->size = range.size;

        shader.setPushConstantRange(range);
    }

    if (!isSampler) {
        switch (uniform->scope) {
            case SHADER_SCOPE_GLOBAL: {
                shader.increaseGlobalSize(uniform->size);
                break;
            }
            case SHADER_SCOPE_INSTANCE: {
                shader.increaseInstanceSize(uniform->size);
            }
            default: break;
        }
    }

    return true;
}

bool ShaderSystem::isUniformNameValid(Shader &shader, const String &name) {
    if (name.isEmpty()) {
        Logger::logError("Uniform name cannot be empty");
        return false;
    }

    return shader.isUniformNameValid(name);
}

bool ShaderSystem::isUniformStateValid(const Shader &shader) {
    if (shader.getState() != SHADER_STATE_NOT_INITIALIZED) {
        Logger::logError("Shader state is invalid for adding uniforms. Uniforms must be added prior to initialization!");
        return false;
    }

    return true;
}

void ShaderSystem::destroyShader(Shader &shader) const {
    backendRef->destroyShader(shader);
    shader.setState(SHADER_STATE_NOT_CREATED);
    shader.destroyTextureMaps();
    shader.clearName();
}

void ShaderSystem::destroyShader(const String &name) {
    const unsigned int shaderId = getId(name);
    if (shaderId == INVALID_ID_U32) return;

    Shader& shader = *assets.getAssetAtIndex(shaderId);

    destroyShader(shader);
}
