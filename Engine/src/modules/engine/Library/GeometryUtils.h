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

    [[nodiscard]] unsigned long getSize() const {return vertexSize;}
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

    void setVertex(const Vertex *vertex, unsigned int index);
};

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

    [[nodiscard]] unsigned long getSize() const {return indexSize;}
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
    Vector3f center{};
    Vector3f minExtent{};
    Vector3f maxExtent{};
};

/**
 * @brief Collection of Geometry utility functions
 */
class GeometryUtils {
private:
    static void reassignIndex(unsigned int indexCount, unsigned int* indices, unsigned int from, unsigned int to);
public:
    static void generateNormals(unsigned int vertexCount, Vertex3d* vertices, unsigned int indexCount, const unsigned int* indices);

    static void generateTangents(unsigned int vertexCount, Vertex *vertices, unsigned int indexCount, void *indices);

    static void filterVertices(unsigned int vertexCount, Vertex *vertices, unsigned int indexCount, void *indices, unsigned int &outVertexCount, DynamicArray
                               <Vertex3d> &outVertices);

    static void destroyConfig(GeometryConfig* config);
};