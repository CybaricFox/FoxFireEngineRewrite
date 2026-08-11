//
// Created by cmorg on 8/6/2026.
//

#include "ShaderSystem.h"

bool ShaderSystem::initialize(const ShaderSystemConfig newConfig, IRendererBackend *backend, ITextureSystem *textureSystem) {
    backendRef = backend;
    textureSystemRef = textureSystem;

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
    for (Shader& shader : assets.getData().getData()) {
        destroyShader(shader);
    }
    assets.shutdown();

    backendRef = nullptr;
    textureSystemRef = nullptr;
}

unsigned int ShaderSystem::getId(const String &shaderName) {
    const unsigned int shaderId = assets.getContext(shaderName)->index;

    if (shaderId == INVALID_ID_U32) {
        Logger::logError("There is no registered shader named " + shaderName);
        return INVALID_ID_U32;
    }

    return shaderId;
}

Shader *ShaderSystem::getShader(const unsigned int shaderId) {
    if (shaderId >= config.maxShaderCount || assets.getData().get(shaderId).getId() == INVALID_ID_U32) {
        return nullptr;
    }

    return &assets.getData().get(shaderId);
}

Shader *ShaderSystem::getShader(const String &shaderName) {
    const unsigned int shaderId = getId(shaderName);
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

bool ShaderSystem::createShader(ShaderConfig& shaderConfig) {
    AssetContext context{};
    Shader* shader = assets.createAsset(shaderConfig.name, context);

    if (!shader->initializeShader(shaderConfig, context.index)) return false;

    unsigned char renderpassId = INVALID_ID_U8;
    if (!backendRef->getRenderpassId(shaderConfig.renderpassName, renderpassId)) {
        Logger::logError("Failed to find renderpass: " + shaderConfig.renderpassName);
        return false;
    }

    if (!backendRef->createShader(*shader, renderpassId, shaderConfig.stageCount, shaderConfig.stageFileNames, shaderConfig.stages)) {
        Logger::logError("Failed to create shader.");
        return false;
    }

    shader->setState(SHADER_STATE_NOT_INITIALIZED);

    for (unsigned int i = 0; i < shaderConfig.attributeCount; i++) {
        addAttribute(*shader, shaderConfig.attributes[i]);
    }

    for (unsigned int i = 0; i < shaderConfig.uniformCount; i++) {
        if (shaderConfig.uniforms[i].type == SHADER_UNIFORM_TYPE_SAMPLER) {
            addSampler(*shader, shaderConfig.uniforms[i]);
        } else {
            addUniform(*shader, shaderConfig.uniforms[i]);
        }
    }

    if (!backendRef->initializeShader(*shader)) {
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
    if (currentShaderId != shaderId) {
        Shader* nextShader = getShader(shaderId);
        if (nextShader == nullptr) return false;
        currentShaderId = shaderId;
        if (!backendRef->useShader(*nextShader)) {
            Logger::logError("Failed to use shader " + nextShader->getName());
            return false;
        }
        if (!backendRef->bindShaderGlobals(*nextShader)) {
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

    Shader& shader = assets.getData().get(currentShaderId);
    return setUniform(getUniformIndex(shader, uniformName), value);
}

bool ShaderSystem::setUniform(const unsigned short index, void *value) {
    Shader& shader = assets.getData().get(currentShaderId);
    ShaderUniform& uniform = shader.getUniform(index);
    if (shader.getBoundScope() != uniform.scope) {
        switch (uniform.scope) {
            case SHADER_SCOPE_GLOBAL: {
                backendRef->bindShaderGlobals(shader);
                break;
            }
            case SHADER_SCOPE_INSTANCE: {
                backendRef->bindShaderInstance(shader, shader.getBoundInstanceId());
                break;
            }
            default: break;
        }
    }

    return backendRef->setUniform(shader, uniform, value);
}

bool ShaderSystem::setSampler(const String &samplerName, Texture &texture) {
    return setUniform(samplerName, &texture);
}

bool ShaderSystem::setSampler(const unsigned short index, Texture &texture) {
    return setUniform(index, &texture);
}

bool ShaderSystem::applyGlobal() {
    return backendRef->applyShaderGlobals(assets.getData().get(currentShaderId));
}

bool ShaderSystem::applyInstance() {
    return backendRef->applyShaderInstance(assets.getData().get(currentShaderId));
}

bool ShaderSystem::bindInstance(const unsigned int instanceId) {
    Shader& shader = assets.getData().get(currentShaderId);
    shader.setBoundInstanceId(instanceId);
    backendRef->bindShaderInstance(shader, instanceId);

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
            Logger::logError("Unrecognized attribute type: " + std::to_string(attributeConfig.type));
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
    if (uniformConfig.scope == SHADER_SCOPE_INSTANCE && !shader.useInstances()) {
        Logger::logError("Cannot add a sampler to a shader that doesn't use instances.");
        return false;
    }

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
            Logger::logError("Shader global texture count exceeds " + std::to_string(config.maxGlobalTextures));
            return false;
        }
        location = globalTextureCount;
        shader.addGlobalTexture(textureSystemRef->getDefaultDiffuseTexture());
    } else {
        if (shader.getInstanceTextureCount() + 1 > config.maxInstanceTextures) {
           Logger::logError("Shader instance texture count exceeds " + std::to_string(config.maxInstanceTextures));
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
        Logger::logError("Number of shader uniforms and samplers cannot exceed " + std::to_string(shader.getUniformCount()));
        return false;
    }

    AssetContext context{};
    ShaderUniform* uniform = shader.createUniform(uniformName, context);
    if (uniform == nullptr) {
        Logger::logError("Failed to create shader uniform " + uniformName);
        return false;
    }

    uniform->index = context.index;
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
        if (uniform->scope == SHADER_SCOPE_LOCAL && !shader.useLocals()) {
            Logger::logError("Cannot add a local uniform to a shader that doesn't use locals.");
            return false;
        }

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
    if (name.empty()) {
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
    shader.clearName();
}

void ShaderSystem::destroyShader(const String &name) {
    const unsigned int shaderId = getId(name);
    if (shaderId == INVALID_ID_U32) return;

    Shader& shader = assets.getData().get(shaderId);

    destroyShader(shader);
}
