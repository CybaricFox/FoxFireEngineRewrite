//
// Created by cmorg on 8/7/2026.
//

#include "ShaderLoader.h"

#include "src/modules/engine/Renderer/Shader.h"

ShaderLoader::ShaderLoader() {
    type = RESOURCE_TYPE_SHADER;
    path = "Shaders";
    memoryTag = RENDER;
}

bool ShaderLoader::load(String name, Resource &outResource, String basePath) {
    if (name.empty()) return false;

    String filePath = basePath + "/" + path + "/" + name + ".FoxShader";

    FileHandler file{};
    if (!file.openFile(filePath, READ, false)) {
        Logger::logError("Failed to open shader file: " + filePath);
        return false;
    }

    outResource.path = filePath;
    auto resourceData = FF_Memory::ff_allocate_class<ShaderConfig>(sizeof(ShaderConfig), RENDER);
    resourceData->attributes.initialize();
    resourceData->uniforms.initialize();
    resourceData->stages.initialize();
    resourceData->stageNames.initialize();
    resourceData->stageFileNames.initialize();

    String line{};
    unsigned long bytesRead = 0;
    unsigned int lineNumber = 0;

    while (file.readLine(line, 511, bytesRead)) {
        lineNumber++;

        StringUtils::trim(line);
        bytesRead = line.length();

        if (bytesRead < 1 || line[0] == '#') continue;

        unsigned int equalsIndex = line.find('=');
        if (equalsIndex == String::npos) {
            Logger::logWarn("Potential formatting issue detected in " + filePath + ". Shader loader could not find '=' on line " + std::to_string(lineNumber));
            continue;
        }

        String variable = line.substr(0, equalsIndex);
        StringUtils::trim(variable);
        String value = line.substr(equalsIndex + 1);
        StringUtils::trim(value);

        if (StringUtils::equalsIgnoreCase(variable, "version")) {

        } else if (StringUtils::equalsIgnoreCase(variable, "name")) {
            resourceData->name = value;
        } else if (StringUtils::equalsIgnoreCase(variable, "renderpass")) {
            resourceData->renderpassName = value;
        }
        else if (StringUtils::equalsIgnoreCase(variable, "stages")) {
            const unsigned int count = StringUtils::recursiveSplit(value, ',', resourceData->stageNames);
            if (resourceData->stageCount == 0) {
                resourceData->stageCount = count;
            } else if (resourceData->stageCount != count) {
                Logger::logError("Invalid file layout. Count mismatch between stage names and file names!");
            }

            for (unsigned char i = 0; i < resourceData->stageCount; i++) {
                if (resourceData->stageNames[i] == "frag" || resourceData->stageNames[i] == "fragment") {
                    resourceData->stages.push(SHADER_STAGE_FRAGMENT);
                } else if (resourceData->stageNames[i] == "vert" || resourceData->stageNames[i] == "vertex") {
                    resourceData->stages.push(SHADER_STAGE_VERTEX);
                } else if (resourceData->stageNames[i] == "geom" || resourceData->stageNames[i] == "geometry") {
                    resourceData->stages.push(SHADER_STAGE_GEOMETRY);
                } else if (resourceData->stageNames[i] == "comp" || resourceData->stageNames[i] == "compute") {
                    resourceData->stages.push(SHADER_STAGE_COMPUTE);
                } else {
                    Logger::logError("Invalid shader file layout. Unrecognized stage: " + resourceData->stageNames[i]);
                }
            }
        } else if (StringUtils::equalsIgnoreCase(variable, "stagefiles")) {
            unsigned int count = StringUtils::recursiveSplit(value, ',', resourceData->stageFileNames);
            if (resourceData->stageCount == 0) {
                resourceData->stageCount = count;
            } else if (resourceData->stageCount != count) {
                Logger::logError("Invalid file layout. Count mismatch between stage names and file names.");
            }
        } else if (StringUtils::equalsIgnoreCase(variable, "use_instance")) {
            StringUtils::stringToBool(value, resourceData->bUseInstances);
        } else if (StringUtils::equalsIgnoreCase(variable, "use_local")) {
            StringUtils::stringToBool(value, resourceData->bUseLocals);
        } else if (StringUtils::equalsIgnoreCase(variable, "attribute")) {
            DynamicArray<String> fields{};
            unsigned int count = StringUtils::recursiveSplit(value, ',', fields);

            if (count != 2) {
                Logger::logError("Invalid file layout! Attribute fields must be 'type,name'!");
            } else {
                ShaderAttributeConfig attribute{};

                if (fields[0] == "float") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_FLOAT32;
                    attribute.size = 4;
                } else if (fields[0] == "Vector2") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_FLOAT32_2;
                    attribute.size = 8;
                } else if (fields[0] == "Vector3") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_FLOAT32_3;
                    attribute.size = 12;
                } else if (fields[0] == "Vector4") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_FLOAT32_4;
                    attribute.size = 16;
                } else if (fields[0] == "Uchar") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_UINT8;
                    attribute.size = 1;
                } else if (fields[0] == "Ushort") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_UINT16;
                    attribute.size = 2;
                } else if (fields[0] == "Uint") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_UINT32;
                    attribute.size = 4;
                } else if (fields[0] == "char") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_INT8;
                    attribute.size = 1;
                } else if (fields[0] == "short") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_INT16;
                    attribute.size = 2;
                } else if (fields[0] == "int") {
                    attribute.type = SHADER_ATTRIBUTE_TYPE_INT32;
                    attribute.size = 4;
                } else {
                    Logger::logError("Invalid file layout! Attribute type " + fields[0] + " is not recognized!");
                    attribute.type = SHADER_ATTRIBUTE_TYPE_FLOAT32;
                    attribute.size = 4;
                }

                attribute.name = fields[1];

                resourceData->attributes.push(attribute);
                resourceData->attributeCount++;
            }

            fields.shutdown();
        } else if (StringUtils::equalsIgnoreCase(variable, "uniform")) {
            DynamicArray<String> fields{};
            unsigned int count = StringUtils::recursiveSplit(value, ',', fields);
            if (count != 3) {
                Logger::logError("Invalid file layout! Uniforms must be 'type,scope,name'!");
            } else {
                ShaderUniformConfig uniform{};
                if (fields[0] == "float") {
                    uniform.type = SHADER_UNIFORM_TYPE_FLOAT32;
                    uniform.size = 4;
                } else if (fields[0] == "Vector2") {
                    uniform.type = SHADER_UNIFORM_TYPE_FLOAT32_2;
                    uniform.size = 8;
                } else if (fields[0] == "Vector3") {
                    uniform.type = SHADER_UNIFORM_TYPE_FLOAT32_3;
                    uniform.size = 12;
                } else if (fields[0] == "Vector4") {
                    uniform.type = SHADER_UNIFORM_TYPE_FLOAT32_4;
                    uniform.size = 16;
                } else if (fields[0] == "Uchar") {
                    uniform.type = SHADER_UNIFORM_TYPE_UINT8;
                    uniform.size = 1;
                } else if (fields[0] == "Ushort") {
                    uniform.type = SHADER_UNIFORM_TYPE_UINT16;
                    uniform.size = 2;
                } else if (fields[0] == "Uint") {
                    uniform.type = SHADER_UNIFORM_TYPE_UINT32;
                    uniform.size = 4;
                } else if (fields[0] == "char") {
                    uniform.type = SHADER_UNIFORM_TYPE_INT8;
                    uniform.size = 1;
                } else if (fields[0] == "short") {
                    uniform.type = SHADER_UNIFORM_TYPE_INT16;
                    uniform.size = 2;
                } else if (fields[0] == "int") {
                    uniform.type = SHADER_UNIFORM_TYPE_INT32;
                    uniform.size = 4;
                } else if (fields[0] == "Matrix4") {
                    uniform.type = SHADER_UNIFORM_TYPE_MATRIX_4;
                    uniform.size = 64;
                } else if (fields[0] == "samp" || fields[0] == "sampler") {
                    uniform.type = SHADER_UNIFORM_TYPE_SAMPLER;
                    uniform.size = 0;
                } else {
                    Logger::logError("Invalid file layout! Uniform type " + fields[0] + " is not recognized!");
                    uniform.type = SHADER_UNIFORM_TYPE_FLOAT32;
                    uniform.size = 4;
                }

                if (fields[1] == "0") {
                    uniform.scope = SHADER_SCOPE_GLOBAL;
                } else if (fields[1] == "1") {
                    uniform.scope = SHADER_SCOPE_INSTANCE;
                } else if (fields[1] == "2") {
                    uniform.scope = SHADER_SCOPE_LOCAL;
                } else {
                    Logger::logError("Invalid file layout! Uniform scope is not recognized: " + fields[1]);
                    uniform.scope = SHADER_SCOPE_GLOBAL;
                }

                uniform.name = fields[2];

                resourceData->uniforms.push(uniform);
                resourceData->uniformCount++;
            }

            fields.shutdown();
        }

        line.clear();
    }

    file.closeFile();

    outResource.data = resourceData;
    outResource.dataSize = sizeof(ShaderConfig);

    return true;
}

void ShaderLoader::unload(Resource &resource) {
    const auto data = static_cast<ShaderConfig *>(resource.data);

    data->stageFileNames.shutdown();
    data->stageNames.shutdown();
    data->stages.shutdown();
    data->attributes.shutdown();
    data->uniforms.shutdown();

    std::destroy_at(data);
    ResourceLoader::unload(resource);
}
