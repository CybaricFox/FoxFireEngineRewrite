//
// Created by cmorg on 8/3/2026.
//

#include "MaterialLoader.h"

MaterialLoader::MaterialLoader() {
    type = RESOURCE_TYPE_MATERIAL;
    path = "Materials";
    memoryTag = MATERIAL;
    memorySize = sizeof(MaterialLoader);
}

bool MaterialLoader::load(const String name, Resource &outResource, const String basePath, ILoaderParameters *params) {
    if (name.isEmpty()) return false;

    const String finalPath = basePath + "/" + path + "/" + name + ".FoxMaterial";

    FileHandler file{};
    if (!file.openFile(finalPath, READ, false)) {
        Logger::logError("Material Loader failed to open material file for reading: " + finalPath);
        return false;
    }

    outResource.path = finalPath;

    const auto resourceData = FF_Memory::ff_allocate<MaterialResourceData>(MATERIAL);
    resourceData->shaderName = "Fox_Fire_Material_Shader";
    resourceData->bAutoRelease = true;
    resourceData->diffuseColor = oneVector4f();
    resourceData->name = name;

    String line{};
    unsigned long bytesRead = 0;
    unsigned int lineNumber = 0;
    while (file.readLine(line, 511, bytesRead)) {
        lineNumber++;

        line.trim();

        //Ignore if line is empty
        if (line.isEmpty()) continue;

        //Ignore comments
        if (line[0] == '#') continue;

        //Find the equal sign on the line if it exists
        const ULong equalIndex = line.find('=');
        if (equalIndex == INVALID_ID_U64 || equalIndex >= line.getSizeInBytes()) {
            Logger::logWarn("Potential format issue found in: " + path + " Failed to find '=' on line" + toString(lineNumber));
            continue;
        }

        //Get the name of the variable on the left and right of the =
        String variable = line.substringLeft(equalIndex - 1);
        String value = line.substringRight(equalIndex + 1);
        variable.trim();
        value.trim();

        if (value.isEmpty()) {continue;}

        //Parse the line
        if (variable == "version") Logger::logDebug("Version: " + value);
        else if (variable == "name") {
            Logger::logDebug("Name: " + value);
            resourceData->name = value;
        }
        else if (variable == "diffuse_color") {
            Logger::logDebug("Diffuse Color: " + value);
            if (!stringToVector4f(value, resourceData->diffuseColor)) {
                Logger::logWarn("Error reading diffuse_color in file: " + path);
            }
        }
        else if (variable == "diffuse_map_name") {
            Logger::logDebug("Diffuse Map Name: " + value);
            resourceData->diffuseName = value;
        }
        else if (variable == "specular_map_name") {
            Logger::logDebug("Specular Map Name: " + value);
            resourceData->specularName = value;
        }
        else if (variable == "normal_map_name") {
            Logger::logDebug("Normal Map Name: " + value);
            resourceData->normalName = value;
        }
        else if (variable == "shader") {
            resourceData->shaderName = value;
        }
        else if (variable == "shine") {
            if (!StringUtils::stringToFloat(value, resourceData->shine)) {
                Logger::logWarn("Error reading the 'shine' value in file: " + finalPath);
                resourceData->shine = 32;
            }
        }

        line.clear();
    }

    file.closeFile();

    outResource.data = resourceData;
    outResource.dataSize = sizeof(MaterialResourceData);
    outResource.name = name;

    return true;
}

void MaterialLoader::unload(Resource &resource) {
    if (resource.data) {
        FF_Memory::ff_free<MaterialResourceData>(resource.data, memoryTag);
        resource.data = nullptr;
        resource.dataSize = 0;
        resource.loaderId = INVALID_ID_U32;
        resource.path.clear();
    }
}
