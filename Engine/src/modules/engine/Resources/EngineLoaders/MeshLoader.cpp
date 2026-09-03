//
// Created by cmorg on 8/22/2026.
//

#include "MeshLoader.h"

#include "src/modules/engine/Library/JsonHandler.h"

bool MeshLoader::importGLTF(FileHandler &file, const String& fileName, DynamicArray<GeometryConfig> &resourceData) {
    JsonHandler json{file};

    //Materials
    DynamicArray<String> materialNames{0};
    if (!createGLTFMaterials(file, json, materialNames)) {
        Logger::logError("Failed to create glTF materials for: " + fileName);
        return false;
    }

    //Process Mesh Data
    DynamicArray<JsonObject>& rawMeshes = *json.getArray("meshes");
    DynamicArray<GLTFMeshSceneData> meshes{rawMeshes.getLength()};

    for (JsonObject& object : rawMeshes) {
        DynamicArray<JsonObject>& primitives = *json.getArray("primitives", &object);

        GLTFMeshSceneData& data = *meshes.emplace();
        data.primitives.initialize(primitives.getLength());

        data.name = json.getString("name", &object);

        for (JsonObject& primitive : primitives) {
            JsonObject& attributes = *json.getObject("attributes", &primitive);
            GLTFMeshPrimitiveData& primitiveData = *data.primitives.emplace();
            primitiveData.positionIndex = json.getInt("POSITION", &attributes);
            primitiveData.normalIndex = json.getInt("NORMAL", &attributes);

            primitiveData.texcoordIndexes.initialize(1);

            unsigned int index = 0;
            while (true) {
                const int result = json.getInt("TEXCOORD_" + std::to_string(index), &attributes);

                if (result == INVALID_ID_U32 / 2) break;

                primitiveData.texcoordIndexes.push(result);
                index++;
            }

            primitiveData.indexIndex = json.getInt("indices", &primitive);
            primitiveData.materialIndex = json.getInt("material", &primitive);
        }
    }

    //Process Accessor data
    DynamicArray<JsonObject>& rawAccessors = *json.getArray("accessors");
    DynamicArray<GLTFMeshAccessorData> accessors{rawAccessors.getLength()};

    for (JsonObject& object : rawAccessors) {
        GLTFMeshAccessorData& data = *accessors.emplace();
        data.bufferView = json.getInt("bufferView", &object);
        data.componentType = json.getInt("componentType", &object);
        data.count = json.getInt("count", &object);

        DynamicArray<JsonObject>* maxArray = json.getArray("max", &object, true);
        DynamicArray<JsonObject>* minArray = json.getArray("min", &object, true);
        if (maxArray) {
            data.minMaxCount = maxArray->getLength();

            for (unsigned int i = 0; i < data.minMaxCount; i++) {
                data.max[i] = json.getFloat("", &(*maxArray)[i]);
                data.min[i] = json.getFloat("", &(*minArray)[i]);
            }
        }

        String type = json.getString("type", &object);
        if (type == "VEC3") {
            data.type = GLTF_TYPE_VEC3;
        } else if (type == "VEC2") {
            data.type = GLTF_TYPE_VEC2;
        } else if (type == "SCALAR") {
            data.type = GLTF_TYPE_SCALAR;
        }
    }

    //Process Buffer View data
    DynamicArray<JsonObject>& rawBufferViews = *json.getArray("bufferViews");
    DynamicArray<GLTFMeshBufferView> bufferViews{rawBufferViews.getLength()};

    for (JsonObject& object : rawBufferViews) {
        GLTFMeshBufferView& data = *bufferViews.emplace();
        data.bufferIndex = json.getInt("buffer", &object);
        data.byteLength = json.getInt("byteLength", &object);
        data.byteOffset = json.getInt("byteOffset", &object);
        data.target = json.getInt("target", &object);
    }

    //Process Buffer data
    DynamicArray<JsonObject>& rawBuffers = *json.getArray("buffers");
    DynamicArray<GLTFMeshBuffer> buffers{rawBuffers.getLength()};

    for (JsonObject& object : rawBuffers) {
        GLTFMeshBuffer& data = *buffers.emplace();
        data.byteSize = json.getInt("byteLength", &object);
        data.fileRef = json.getString("uri", &object);
    }

    //Read the binary file
    FileHandler binFile{};

    for (GLTFMeshBuffer& buffer : buffers) {
        if (buffer.fileRef.empty()) continue;

        String binPath = StringUtils::getDirectoryFromPath(fileName) + "/" + path + "/" + buffer.fileRef;
        if (!binFile.openFile(binPath, READ, true)) {
            Logger::logError("Failed to open glTF buffer: " + binPath);
            binFile.closeFile();
            return false;
        }

        buffer.data = static_cast<unsigned char *>(FF_Memory::ff_allocate(buffer.byteSize, RESOURCE));

        unsigned long bytesRead = 0;
        if (!binFile.read(buffer.byteSize, buffer.data, bytesRead)) {
            Logger::logError("Failed to read glTF buffer: " + buffer.fileRef);
            FF_Memory::ff_free(buffer.data, buffer.byteSize, RESOURCE);
            buffer.data = nullptr;
            binFile.closeFile();
            return false;
        }

        binFile.closeFile();
    }

    //Final processing
    resourceData.initialize(meshes.getLength());
    for (GLTFMeshSceneData& mesh : meshes) {
        for (GLTFMeshPrimitiveData& primitive : mesh.primitives) {
            GLTFMeshDataContext positionContext{};
            GLTFMeshDataContext normalContext{};
            GLTFMeshDataContext texCoordContext{};
            GLTFMeshDataContext indexContext{};

            bool skipNormals = mesh.primitives[0].normalIndex == INVALID_ID_U32;
            bool skipTexCoords = mesh.primitives[0].texcoordIndexes.getLength() == 0;

            //position data
            GLTFMeshAccessorData& positionAccessor = accessors[mesh.primitives[0].positionIndex];
            GLTFMeshBufferView &positionBufferView = bufferViews[positionAccessor.bufferView];
            GLTFMeshBuffer &positionBuffer = buffers[positionBufferView.bufferIndex];
            positionContext = processGLTFObject(positionAccessor, positionBufferView, positionBuffer);
            if (!positionContext.bIsValid) {
                Logger::logError("Position data for mesh " + StringUtils::getFilenameNoExtensionFromPath(fileName) + " is invalid!");
                return false;
            }

            //Normals data
            if (!skipNormals) {
                GLTFMeshAccessorData& normalAccessor = accessors[mesh.primitives[0].normalIndex];
                GLTFMeshBufferView &nomralBufferView = bufferViews[normalAccessor.bufferView];
                GLTFMeshBuffer &normalBuffer = buffers[nomralBufferView.bufferIndex];
                normalContext = processGLTFObject(normalAccessor, nomralBufferView, normalBuffer);
                if (!normalContext.bIsValid) {
                    Logger::logError("Normal data for mesh " + StringUtils::getFilenameNoExtensionFromPath(fileName) + " is invalid!");
                    return false;
                }
            }

            //Texture coordinate data
            if (!skipTexCoords) {
                GLTFMeshAccessorData& texCoordAccessor = accessors[mesh.primitives[0].texcoordIndexes[0]];
                GLTFMeshBufferView &texCoordBufferView = bufferViews[texCoordAccessor.bufferView];
                GLTFMeshBuffer &texCoordBuffer = buffers[texCoordBufferView.bufferIndex];
                texCoordContext = processGLTFObject(texCoordAccessor, texCoordBufferView, texCoordBuffer);
                if (!texCoordContext.bIsValid) {
                    Logger::logError("Texture Coordinate data for mesh " + StringUtils::getFilenameNoExtensionFromPath(fileName) + " is invalid!");
                    return false;
                }
            }

            //Index data
            GLTFMeshAccessorData& indexAccessor = accessors[mesh.primitives[0].indexIndex];
            GLTFMeshBufferView &indexBufferView = bufferViews[indexAccessor.bufferView];
            GLTFMeshBuffer &indexBuffer = buffers[indexBufferView.bufferIndex];
            indexContext = processGLTFObject(indexAccessor, indexBufferView, indexBuffer);
            if (!indexContext.bIsValid) {
                Logger::logError("Index data for mesh " + StringUtils::getFilenameNoExtensionFromPath(fileName) + " is invalid!");
                return false;
            }

            //Create Geometry
            GeometryConfig &config = *resourceData.emplace();
            processExtents(config, positionAccessor);

            //Materials
            config.materialName = materialNames[primitive.materialIndex];

            //Vertices
            config.vertices.initialize<Vertex3d>(positionAccessor.count);
            for (unsigned int i = 0; i < positionAccessor.count; i++) {
                Vertex3d &vertex = *static_cast<Vertex3d *>(config.vertices.getVertex(i));
                unsigned char *positionAddress = positionContext.data + (positionContext.stride * i);
                unsigned char* normalAddress = normalContext.data + (normalContext.stride * i);
                unsigned char* texCoordAddress = texCoordContext.data + (texCoordContext.stride * i);

                float position[3]{};
                FF_Memory::ff_copy(position, positionAddress, sizeof(float) * 3);
                vertex.position = {position[0], position[1], position[2]};

                if (skipNormals) {
                    vertex.normal = {0, 0, 1};
                } else {
                    float normal[3]{};
                    FF_Memory::ff_copy(normal, normalAddress, sizeof(float) * 3);
                    vertex.normal = {normal[0], normal[1], normal[2]};
                }

                if (skipTexCoords) {
                    vertex.textureCoordinate = zeroVector2f();
                } else {
                    float texCoord[2]{};
                    FF_Memory::ff_copy(texCoord, texCoordAddress, sizeof(float) * 2);
                    texCoord[1] = 1 - texCoord[1]; //Invert the texCoord since all textures are inverted
                    vertex.textureCoordinate = {texCoord[0], texCoord[1]};
                }

                vertex.color = oneVector4f();
            }

            //Indices
            config.indices.initialize<unsigned int>(indexAccessor.count);
            for (unsigned int i = 0; i < indexAccessor.count; i++) {
                unsigned char *indexAddress = indexContext.data + (indexContext.stride * i);
                unsigned int index = INVALID_ID_U32;
                switch (indexAccessor.componentType) {
                    case 5121: {
                        unsigned char value = 0;

                        FF_Memory::ff_copy(
                            &value,
                            indexAddress,
                            sizeof(value)
                        );

                        index = value;
                        break;
                    }

                    case 5123: {
                        unsigned short value = 0;

                        FF_Memory::ff_copy(
                            &value,
                            indexAddress,
                            sizeof(value)
                        );

                        index = value;
                        break;
                    }

                    case 5125: {
                        unsigned int value = 0;

                        FF_Memory::ff_copy(
                            &value,
                            indexAddress,
                            sizeof(value)
                        );

                        index = value;
                        break;
                    }

                    default: {
                        Logger::logError(
                            "Unsupported glTF index component type."
                        );
                        return false;
                    }
                }
                config.indices.setIndex(index, i);
            }

            unsigned int outCount = 0;
            auto newVertices = FF_Memory::ff_allocate_class<DynamicArray<Vertex3d>>(sizeof(DynamicArray<Vertex3d>), DYNAMIC_ARRAY);
            newVertices->initialize(0);
            GeometryUtils::filterVertices(config.vertices.getCount(), config.vertices.getVertex(0), config.indices.getCount(), config.indices.getIndex(0), outCount, *newVertices);

            //If count goes down, reassign the vertices.
            if (config.vertices.getCount() > outCount) {
                config.vertices.shutdown();
                config.vertices.initialize<Vertex3d>(outCount);
                for (unsigned int i = 0; i < outCount; i++) {
                    config.vertices.setVertex(&(*newVertices)[i], i);
                }
            }

            GeometryUtils::generateTangents(config.vertices.getCount(), config.vertices.getVertex(0), config.indices.getCount(), config.indices.getIndex(0));

            FF_Memory::ff_free_class<DynamicArray<Vertex3d>>(newVertices, sizeof(DynamicArray<Vertex3d>), DYNAMIC_ARRAY);
        }
    }

    json.shutdown();

    for (GLTFMeshSceneData& mesh : meshes) {
        for (GLTFMeshPrimitiveData& primitive : mesh.primitives) {
            primitive.texcoordIndexes.shutdown();
        }

        mesh.primitives.shutdown();
    }

    for (GLTFMeshBuffer& buffer : buffers) {
        FF_Memory::ff_free(buffer.data, buffer.byteSize, RESOURCE);
    }

    return writeFoxMesh(fileName, StringUtils::getFilenameFromPath(fileName), resourceData.getLength(), resourceData);
}

GLTFMeshDataContext MeshLoader::processGLTFObject(const GLTFMeshAccessorData &accessorData, const GLTFMeshBufferView &bufferView, const GLTFMeshBuffer &buffer) {
    GLTFMeshDataContext context{};
    context.data = buffer.data + bufferView.byteOffset + 0;
    //Replace 0 with accessor offset when it becomes relevant.

    switch (accessorData.componentType) {
        case 5120:
        case 5121:
            context.componentSize = 1;
            break;
        case 5122:
        case 5123:
            context.componentSize = 2;
            break;
        case 5125:
        case 5126:
            context.componentSize = 4;
            break;
        default:
            Logger::logError("Unknown glTF component type.");
            return context;
    }

    switch (accessorData.type) {
        case GLTF_TYPE_SCALAR:
            context.componentCount = 1;
            break;

        case GLTF_TYPE_VEC2:
            context.componentCount = 2;
            break;

        case GLTF_TYPE_VEC3:
            context.componentCount = 3;
            break;
        default:
            Logger::logError("Unknown glTF type.");
            return context;
    }

    context.elementSize = context.componentSize * context.componentCount;
    context.stride = bufferView.byteStride != 0 ? bufferView.byteStride : context.elementSize;

    context.bIsValid = true;
    return context;
}

void MeshLoader::processExtents(GeometryConfig &config, const GLTFMeshAccessorData& positionData) {
    config.maxExtent = {positionData.max[0], positionData.max[1], positionData.max[2]};
    config.minExtent = {positionData.min[0], positionData.min[1], positionData.min[2]};

    for (unsigned char i = 0; i < 3; i++) {
        config.center.elements[i] = (config.minExtent.elements[i] + config.maxExtent.elements[i]) / 2;
    }
}

bool MeshLoader::createGLTFMaterials(FileHandler &file, JsonHandler& json, DynamicArray<String> &materialNames) {
    DynamicArray<JsonObject>& rawImages = *json.getArray("images");
    DynamicArray<GLTFMeshImage> images{rawImages.getLength()};

    for (unsigned long i = 0; i < rawImages.getLength(); i++) {
        GLTFMeshImage& image = *images.emplace();
        JsonObject& rawImage = rawImages[i];

        image.mimeType = json.getString("mimeType", &rawImage);
        image.name = json.getString("name", &rawImage);
        image.fileRef = json.getString("uri", &rawImage);
    }

    DynamicArray<JsonObject>& rawTextures = *json.getArray("textures");
    DynamicArray<GLTFMeshTexture> textures{rawTextures.getLength()};

    for (unsigned long i = 0; i < rawTextures.getLength(); i++) {
        GLTFMeshTexture& texture = *textures.emplace();
        JsonObject& rawTexture = rawTextures[i];

        texture.sampler = json.getInt("sampler", &rawTexture);
        texture.source = json.getInt("source", &rawTexture);
    }

    DynamicArray<JsonObject>& rawMaterials = *json.getArray("materials");
    DynamicArray<GLTFMeshMaterial> materials{rawMaterials.getLength()};

    for (unsigned int i = 0; i < rawMaterials.getLength(); i++) {
        JsonObject& rawMaterial = rawMaterials[i];
        JsonObject& rawPBR = *json.getObject("pbrMetallicRoughness", &rawMaterial);
        JsonObject& rawColorTexture = *json.getObject("baseColorTexture", &rawPBR);
        GLTFMeshMaterial& material = *materials.emplace();

        material.alphaMode = json.getString("alphaMode", &rawMaterial, true);
        json.getBool("doubleSided", material.doubleSided, &rawMaterial);
        material.name = json.getString("name", &rawMaterial);
        material.pbr.colorTexture.index = json.getInt("index", &rawColorTexture);
        material.pbr.metallic = json.getInt("metallicFactor", &rawPBR);
        material.pbr.roughness = json.getFloat("roughnessFactor", &rawPBR);
    }

    //Create materials
    for (unsigned int i = 0; i < materials.getLength(); i++) {
        //Skip this step if the material file already exists
        if (file.exists("Assets/Materials/" + images[i].name + "_Material.FoxMaterial")) continue;

        MaterialResourceData materialResourceData{};
        unsigned int textureIndex = materials[i].pbr.colorTexture.index;
        GLTFMeshTexture& texture = textures[textureIndex];
        GLTFMeshImage& image = images[texture.source];
        String name = image.fileRef;
        unsigned int index = name.find('.');
        name = name.substr(0, index);


        materialResourceData.name = name + "_Material";
        materialResourceData.diffuseName = name;
        materialResourceData.specularName = name + "_SPEC";
        materialResourceData.normalName = name + "_NORM";
        materialResourceData.diffuseColor = createVector4f(0, 0, 0, 1);
        materialResourceData.bAutoRelease = true;
        materialResourceData.shine = 8; //GLTF does not contain this. Default it to 8.
        materialResourceData.shaderName = "Fox_Fire_Material_Shader"; //Default material shader

        writeFoxMaterial("Assets/Materials", materialResourceData);
        materialNames.push(materialResourceData.name);
    }

    return true;
}

bool MeshLoader::loadFoxMesh(FileHandler &file, DynamicArray<GeometryConfig> &outConfigs) {
    //Version
    unsigned long inBytes = 0;
    unsigned short versionA = 0;
    unsigned short versionB = 0;
    unsigned short versionC = 0;

    file.read(sizeof(unsigned short), &versionA, inBytes);
    file.read(sizeof(unsigned short), &versionB, inBytes);
    file.read(sizeof(unsigned short), &versionC, inBytes);

    //Name
    unsigned int length = 0;
    file.read(sizeof(unsigned int), &length, inBytes);
    char name[256]{};
    file.read(sizeof(char) * length, &name, inBytes);

    //Geometry Count
    unsigned int geometryCount = 0;
    file.read(sizeof(unsigned int), &geometryCount, inBytes);

    outConfigs.initialize(geometryCount);
    for (unsigned int i = 0; i < geometryCount; i++) {
        GeometryConfig& config = *outConfigs.emplace();

        //Vertices
        unsigned int count = 0;
        unsigned int size = 0;
        file.read(sizeof(unsigned int), &size, inBytes);
        file.read(sizeof(unsigned int), &count, inBytes);
        config.vertices.initialize<Vertex3d>(count);
        file.read(size * count, config.vertices.getVertex(0), inBytes);

        //Indices
        file.read(sizeof(unsigned int), &size, inBytes);
        file.read(sizeof(unsigned int), &count, inBytes);
        config.indices.initialize<unsigned int>(count);
        file.read(size * count, config.indices.getIndex(0), inBytes);

        //Name
        unsigned int configLength = 0;
        char configName[256]{};
        file.read(sizeof(unsigned int), &configLength, inBytes);
        file.read(sizeof(char) * configLength, &configName, inBytes);
        config.name = configName;

        //material name
        file.read(sizeof(unsigned int), &configLength, inBytes);
        file.read(sizeof(char) * configLength, &configName, inBytes);
        config.materialName = configName;

        //Center
        file.read(sizeof(Vector3f), &config.center, inBytes);

        //Extents
        file.read(sizeof(Vector3f), &config.minExtent, inBytes);
        file.read(sizeof(Vector3f), &config.maxExtent, inBytes);
    }

    file.closeFile();
    return true;
}

bool MeshLoader::writeFoxMesh(const String &path, const String &name, const unsigned int geometryCount, DynamicArray<GeometryConfig>& geometries) {
    FileHandler file{};

    if (file.exists(path)) {
        Logger::logInfo(name + " already has a FoxMesh! The file will be overwritten, but this could be a sign of an error.");
    }

    if (!file.openFile(path, WRITE, true)) {
        Logger::logError("Failed to create file: " + path);
        return false;
    }

    unsigned long outBytes = 0;

    //Version
    constexpr unsigned short versionA = 0x0001U;
    constexpr unsigned short versionB = 0x0000U;
    constexpr unsigned short versionC = 0x0000U;

    file.write(sizeof(unsigned short), &versionA, outBytes);
    file.write(sizeof(unsigned short), &versionB, outBytes);
    file.write(sizeof(unsigned short), &versionC, outBytes);

    //Name
    unsigned int length = name.length();
    file.write(sizeof(unsigned int), &length, outBytes);
    file.write(sizeof(char) * length, name.c_str(), outBytes);

    //Geometry Count
    file.write(sizeof(unsigned int), &geometryCount, outBytes);

    //Geometry
    for (GeometryConfig &geometryConfig : geometries) {
        //Vertices
        unsigned int count = geometryConfig.vertices.getCount();
        unsigned int size = geometryConfig.vertices.getSize();
        file.write(sizeof(unsigned int), &size, outBytes);
        file.write(sizeof(unsigned int), &count, outBytes);
        file.write(size * count, geometryConfig.vertices.getVertex(0), outBytes);

        //Indices
        count = geometryConfig.indices.getCount();
        size = geometryConfig.indices.getSize();
        file.write(sizeof(unsigned int), &size, outBytes);
        file.write(sizeof(unsigned int), &count, outBytes);
        file.write(size * count, geometryConfig.indices.getIndex(0), outBytes);

        //Geometry name
        length = geometryConfig.name.length();
        file.write(sizeof(unsigned int), &length, outBytes);
        file.write(sizeof(char) * length, geometryConfig.name.c_str(), outBytes);

        //Material name
        length = geometryConfig.materialName.length();
        file.write(sizeof(unsigned int), &length, outBytes);
        file.write(sizeof(char) * length, geometryConfig.materialName.c_str(), outBytes);

        //Center
        file.write(sizeof(Vector3f), &geometryConfig.center, outBytes);

        //Extents
        file.write(sizeof(Vector3f), &geometryConfig.minExtent, outBytes);
        file.write(sizeof(Vector3f), &geometryConfig.maxExtent, outBytes);
    }

    file.closeFile();

    return true;
}

bool MeshLoader::writeFoxMaterial(const String &directory, const MaterialResourceData &config) {
    FileHandler file{};

    const String finalPath = directory + "/" + config.name + ".FoxMaterial";
    if (!file.openFile(finalPath, WRITE, true)) {
        Logger::logError("Failed to create file: " + finalPath);
        return false;
    }

    bool result = file.writeLine("#Comment");
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine(" ");
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine("version = 1.0.0");
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine("name = " + config.name);
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    String diffuseString{};
    vector4fToString(diffuseString, config.diffuseColor);
    result = file.writeLine("diffuse_color = " + diffuseString);
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine("shine = " + std::to_string(config.shine));
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine("diffuse_map_name = " + config.diffuseName);
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine("specular_map_name = " + config.specularName);
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine("normal_map_name = " + config.normalName);
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }
    result = file.writeLine("shader = " + config.shaderName);
    if (!result) {
        Logger::logError("Failed to write file: " + finalPath);
        file.closeFile();
        return false;
    }

    /*//Version block
    unsigned long outBytes = 0;

    constexpr unsigned short versionA = 0x0001U;
    constexpr unsigned short versionB = 0x0000U;
    constexpr unsigned short versionC = 0x0000U;
    file.write(sizeof(unsigned short), &versionA, outBytes);
    file.write(sizeof(unsigned short), &versionB, outBytes);
    file.write(sizeof(unsigned short), &versionC, outBytes);

    //Name block
    unsigned int length = config.name.length();
    file.write(sizeof(unsigned int), &length, outBytes);
    file.write(sizeof(char) * length, config.name.c_str(), outBytes);

    //texture names
    length = config.diffuseName.length();
    file.write(sizeof(unsigned int), &length, outBytes);
    file.write(sizeof(char) * length, config.diffuseName.c_str(), outBytes);

    length = config.specularName.length();
    file.write(sizeof(unsigned int), &length, outBytes);
    file.write(sizeof(char) * length, config.specularName.c_str(), outBytes);

    length = config.normalName.length();
    file.write(sizeof(unsigned int), &length, outBytes);
    file.write(sizeof(char) * length, config.normalName.c_str(), outBytes);

    //Shader name
    length = config.shaderName.length();
    file.write(sizeof(unsigned int), &length, outBytes);
    file.write(sizeof(char) * length, config.shaderName.c_str(), outBytes);

    //Diffuse Color block
    file.write(sizeof(Vector4f), &config.diffuseColor, outBytes);

    //Shine
    file.write(sizeof(float), &config.shine, outBytes);*/

    file.closeFile();
    return true;
}

MeshLoader::MeshLoader() {
    type = RESOURCE_TYPE_MESH;
    path = "Models";
    memoryTag = RESOURCE;
    memorySize = sizeof(MeshLoader);
}

bool MeshLoader::load(const String name, Resource &outResource, const String basePath) {
    if (name.empty()) return false;

    String finalPath = basePath + "/" + path + "/" + name;
    FileHandler file{};
    MeshFileType type = MESH_FILE_TYPE_UNKNOWN;

    constexpr unsigned int SUPPORTED_FILE_TYPE_COUNT = 2;
    MeshFileContext supportedTypes[SUPPORTED_FILE_TYPE_COUNT]{};
    supportedTypes[0] = {".FoxMesh", MESH_FILE_TYPE_FOXMESH, true};
    supportedTypes[1] = {".gltf", MESH_FILE_TYPE_GLTF, false};

    for (unsigned int i = 0; i < SUPPORTED_FILE_TYPE_COUNT; i++) {
        if (!file.exists(finalPath + supportedTypes[i].extension)) continue;

        if (file.openFile(finalPath + supportedTypes[i].extension, READ, supportedTypes[i].bIsBinary)) {
            finalPath = finalPath + supportedTypes[i].extension;
            type = supportedTypes[i].type;
            break;
        }
    }

    if (type == MESH_FILE_TYPE_UNKNOWN) {
        Logger::logError("Failed to load mesh file: " + finalPath + " No valid extension was found!");
        return false;
    }

    outResource.path = finalPath;
    DynamicArray<GeometryConfig>* resourceData = FF_Memory::ff_allocate_class<DynamicArray<GeometryConfig>>(sizeof(DynamicArray<GeometryConfig>), DYNAMIC_ARRAY);

    bool result = false;
    switch (type) {
        case MESH_FILE_TYPE_GLTF: {
            String fileName = basePath + "/" + path + "/" + name + ".FoxMesh";
            result = importGLTF(file, fileName, *resourceData);
            break;
        }
        case MESH_FILE_TYPE_FOXMESH: {
            result = loadFoxMesh(file, *resourceData);
            break;
        }
        case MESH_FILE_TYPE_UNKNOWN:
        default: {
            Logger::logError("Failed to load mesh file: " + finalPath);
            result = false;
            break;
        }
    }

    file.closeFile();

    if (!result) {
        Logger::logError("Failed to process mesh file: " + finalPath);
        resourceData->shutdown();
        FF_Memory::ff_free_class<DynamicArray<GeometryConfig>>(resourceData, sizeof(DynamicArray<GeometryConfig>), DYNAMIC_ARRAY);
        outResource.data = nullptr;
        outResource.dataSize = 0;
        return false;
    }

    outResource.data = resourceData;
    outResource.dataSize = resourceData->getLength();

    return true;
}

void MeshLoader::unload(Resource &resource) {
    const auto array = static_cast<DynamicArray<GeometryConfig> *>(resource.data);

    for (GeometryConfig& config : *array) {
        GeometryUtils::destroyConfig(&config);
    }

    array->shutdown();
    FF_Memory::ff_free_class<DynamicArray<GeometryConfig>>(resource.data, sizeof(DynamicArray<GeometryConfig>), DYNAMIC_ARRAY);
    resource.data = nullptr;
    resource.dataSize = 0;
}

