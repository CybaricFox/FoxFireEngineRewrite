//
// Created by cmorg on 8/11/2026.
//

#pragma once
#include "FF_Math.h"
#include "src/modules/engine/Memory/LinearAllocator.h"

/**
 *  @file GeometryUtils.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/11/2026
 *
 *  @copyright (c) 2026
 */

/**
 * @brief Contains all data relate to vertices in GeometryConfig. Vertices are stored next to eachother in memory.
 */
class VertexGeometryData {
private:
    /** @brief size of the vertex type */
    unsigned long vertexSize = 0;
    /** @brief number of vertices */
    unsigned int vertexCount = 0;
    LinearAllocator allocation{};
    DynamicArray<Vertex*> vertices{};

public:
    template <typename T>
    void initialize(const unsigned int count) {
        if (vertexCount != 0) {
            Logger::logError("Vertex Geometry Data is already initialized!");
            return;
        }

        vertexCount = count;
        vertexSize = sizeof(T);

        vertices.initialize(count);
        allocation.initialize(vertexCount * vertexSize);

        for (unsigned int i = 0; i < vertexCount; i++) {
            T* block = static_cast<T *>(allocation.allocate(vertexSize));
            std::construct_at(block);
            vertices.push(block);
        }
    }
    void shutdown();

    /**
     * @brief Returns the size of the vertices
     * @return
     */
    [[nodiscard]] unsigned long getSize() const {return vertexSize;}
    /**
     * @brief Returns the number of vertices.
     * @return
     */
    [[nodiscard]] unsigned long getCount() const {return vertexCount;}
    /**
     * @brief Fetches the vertex struct at that index. Fetching 0 also returns the beginning of the vertex array.
     * @param i index of the vertex
     * @return Pointer to the vertex at that index
     */
    Vertex* getVertex(const unsigned int i) {
        if (i > vertices.getLength()) {
            Logger::logError("Vertex index out of range!");
            return nullptr;
        }
        return vertices[i];
    }

    /**
     * @brief Sets the vertex at the given index.
     * @param vertex The new vertex to set.
     * @param index The index to set it at.
     */
    void setVertex(const Vertex *vertex, unsigned int index);
};

/**
 * @brief Contains all data relate to indices in GeometryConfig. Indices are stored next to eachother in memory.
 */
class IndexGeometryData {
private:
    /** @brief size of the vertex type */
    unsigned long indexSize = 0;
    /** @brief number of vertices */
    unsigned int indexCount = 0;
    LinearAllocator allocation{};
    DynamicArray<void*> indicies{};

public:
    template <typename T>
    void initialize(const unsigned int count) {
        if (indexCount != 0) {
            Logger::logError("Vertex Geometry Data is already initialized!");
            return;
        }

        indexCount = count;
        indexSize = sizeof(T);

        indicies.initialize(count);
        allocation.initialize(indexCount * indexSize);

        for (unsigned int i = 0; i < indexCount; i++) {
            T* block = static_cast<T *>(allocation.allocate(indexSize));
            indicies.push(block);
        }
    }
    void shutdown();

    /**
     * @brief Returns the size of the indices.
     * @return
     */
    [[nodiscard]] unsigned long getSize() const {return indexSize;}
    /**
     * @brief Returns the number of indices.
     * @return
     */
    [[nodiscard]] unsigned long getCount() const {return indexCount;}
    /**
     * @brief Fetches the vertex struct at that index. Fetching 0 also returns the beginning of the vertex array.
     * @param i index of the vertex
     * @return Pointer to the vertex at that index
     */
    void* getIndex(const unsigned int i) {return indicies[i];}

    /**
     * @brief Sets the value at the index
     * @param value Value to set
     * @param index Index of the target in the array.
     */
    void setIndex(unsigned long value, unsigned int index);
};

/**
 * @brief Holds config data for a piece of geometry
 */
struct GeometryConfig {
    /** @brief Vertex data */
    VertexGeometryData vertices{};
    /** @brief Index data */
    IndexGeometryData indices{};
    /** @brief Name of the geometry */
    String name{};
    /** @brief name of the material */
    String materialName{};
    /** @brief Path to the material */
    String materialPath{};
    /**
     * @brief Center of the geometry.
     */
    Vector3f center{};
    /**
     * @brief
     */
    Vector3f minExtent{};
    /**
     * @brief
     */
    Vector3f maxExtent{};
};

/**
 * @brief Collection of Geometry utility functions
 */
class GeometryUtils {
private:
    static void reassignIndex(unsigned int indexCount, unsigned int* indices, unsigned int from, unsigned int to);
public:
    /**
     * @brief Generates normals for a geometry.
     * @param vertexCount Number of vertices
     * @param vertices Pointer to the array of vertices
     * @param indexCount Number of incides
     * @param indices Pointer to the array of indices
     */
    static void generateNormals(unsigned int vertexCount, Vertex3d* vertices, unsigned int indexCount, const unsigned int* indices);

    /**
     * @brief Generates tangents for a geometry.
     * @param vertexCount Number of vertices
     * @param vertices Pointer to the array of vertices
     * @param indexCount Number of incides
     * @param indices Pointer to the array of indices
     */
    static void generateTangents(unsigned int vertexCount, Vertex *vertices, unsigned int indexCount, void *indices);

    /**
     * @brief Compacts vertices with the same locations.
     * @param vertexCount
     * @param vertices
     * @param indexCount
     * @param indices
     * @param outVertexCount Vertex count after compacting.
     * @param outVertices Pointer to the new vertex array.
     */
    static void filterVertices(unsigned int vertexCount, Vertex *vertices, unsigned int indexCount, void *indices, unsigned int &outVertexCount, DynamicArray<Vertex3d> &outVertices);

    /**
     * @brief Destroys a geometry config.
     * @param config
     */
    static void destroyConfig(GeometryConfig* config);
};