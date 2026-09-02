//
// Created by cmorg on 8/22/2026.
//

#pragma once
#include "src/modules/engine/Library/JsonHandler.h"
#include "src/modules/engine/Renderer/IGeometrySystem.h"
#include "src/modules/engine/Resources/ResourceLoader.h"

/**
 *  @file MeshLoader.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/22/2026
 *
 *  @copyright (c) 2026
 */

enum MeshFileType {
    MESH_FILE_TYPE_UNKNOWN,
    MESH_FILE_TYPE_FOXMESH,
    MESH_FILE_TYPE_GLTF
    //OBJ will not be supported by default due to gltf being a better alternative
};

struct MeshFileContext {
    String extension{};
    MeshFileType type = MESH_FILE_TYPE_UNKNOWN;
    bool bIsBinary = false;
};

enum GLTFType {
    GLTF_TYPE_UNKNOWN,
    GLTF_TYPE_VEC3,
    GLTF_TYPE_VEC2,
    GLTF_TYPE_SCALAR
};

struct MeshPrimitiveData {
    unsigned int positionIndex = INVALID_ID_U32;
    unsigned int normalIndex = INVALID_ID_U32;
    unsigned int materialIndex = INVALID_ID_U32;
    unsigned int indexIndex = INVALID_ID_U32;
    DynamicArray<unsigned int> texcoordIndexes{};
};

struct MeshSceneData {
    String name{};
    DynamicArray<MeshPrimitiveData> primitives{};
};

struct MeshAccessorData {
    unsigned int bufferView = INVALID_ID_U32;
    unsigned int componentType = INVALID_ID_U32;
    unsigned int count = 0;
    unsigned char minMaxCount = 0;
    float max[3]{};
    float min[3]{};
    GLTFType type = GLTF_TYPE_UNKNOWN;
};

struct MeshBufferView {
    unsigned int bufferIndex = INVALID_ID_U32;
    unsigned long byteLength = 0;
    unsigned long byteOffset = 0;
    unsigned long byteStride = 0;
    unsigned int target = INVALID_ID_U32;
};

struct MeshSampler {
    int magFilter = static_cast<int>(INVALID_ID_U32);
    int minFilter = static_cast<int>(INVALID_ID_U32);
};

struct MeshBuffer {
    unsigned long byteSize = 0;
    String fileRef{};
    unsigned char* data = nullptr;
};

struct MeshDataContext {
    bool bIsValid = false;
    unsigned char* data = nullptr;
    unsigned int componentSize = 0;
    unsigned int componentCount = 0;
    unsigned long elementSize = 0;
    unsigned long stride = 0;
};

struct MeshImage {
    String mimeType{};
    String name{};
    String fileRef{};
};

struct MeshTexture {
    unsigned int sampler = INVALID_ID_U32;
    unsigned int source = INVALID_ID_U32;
};

struct MeshColorTexture {
    unsigned int index = INVALID_ID_U32;
};

struct MeshPBR {
    MeshColorTexture colorTexture{};
    int metallic = 0;
    float roughness = 0;
};

struct MeshMaterial {
    bool doubleSided = false;
    String name{};
    MeshPBR pbr{};
    String alphaMode{};
};

class MeshLoader final : public ResourceLoader{
private:
    bool importGLTF(FileHandler &file, const String& fileName, DynamicArray<GeometryConfig>& resourceData);

    MeshDataContext processGLTFObject(const MeshAccessorData &accessorData, const MeshBufferView &bufferView, const MeshBuffer &buffer);
    void processExtents(GeometryConfig &config, const MeshAccessorData &positionData);
    bool createGLTFMaterials(FileHandler &file, JsonHandler &json, DynamicArray<String> &materialNames);
    bool loadFoxMesh(FileHandler& file, DynamicArray<GeometryConfig>& outConfigs);
    bool writeFoxMesh(String path, String name, unsigned int geometryCount, DynamicArray<GeometryConfig> &geometries);
    bool writeFoxMaterial(const String &directory, const MaterialResourceData& config);
public:
    MeshLoader();

    bool load(String name, Resource &outResource, String basePath) override;
    void unload(Resource &resource) override;
};