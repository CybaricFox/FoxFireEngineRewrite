//
// Created by cmorg on 8/3/2026.
//

#include "MaterialLoader.h"

MaterialLoader::MaterialLoader() {
    type = RESOURCE_TYPE_MATERIAL;
    path = "Materials";
    memoryTag = MATERIAL;
}

bool MaterialLoader::load(const String name, Resource &outResource, const String basePath) {
    if (name.empty()) return false;

    const String finalPath = basePath + "/" + path + "/" + name + ".FoxMaterial";

    FileHandler file{};
    if (!file.openFile(finalPath, READ, false)) {
        Logger::logError("Material Loader failed to open material file for reading: " + finalPath);
        return false;
    }

    outResource.path = finalPath;

    const auto resourceData = static_cast<MaterialResourceData *>(FF_Memory::ff_allocate(sizeof(MaterialResourceData), MATERIAL));
    resourceData->shaderName = "Fox_Fire_Material_Shader";
    resourceData->bAutoRelease = true;
    resourceData->diffuseColor = oneVector4f();
    resourceData->name = name;

    String line{};
    unsigned long bytesRead = 0;
    unsigned int lineNumber = 0;
    while (file.readLine(line, 511, bytesRead)) {
        lineNumber++;

        StringUtils::trim(line);

        //Ignore if line is empty
        if (line.empty()) continue;

        //Ignore comments
        if (line[0] == '#') continue;

        //Find the equal sign on the line if it exists
        const unsigned long equalIndex = line.find('=');
        if (equalIndex == String::npos || equalIndex >= line.length()) {
            Logger::logWarn("Potential format issue found in: " + path + " Failed to find '=' on line" + std::to_string(lineNumber));
            continue;
        }

        //Get the name of the variable on the left and right of the =
        String variable = line.substr(0, equalIndex);
        String value = line.substr(equalIndex + 1);
        StringUtils::trim(variable);
        StringUtils::trim(value);

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
