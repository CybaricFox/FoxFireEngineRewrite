//
// Created by cmorg on 8/11/2026.
//

#include "GeometryUtils.h"

void VertexGeometryData::shutdown() {
    allocation.shutdown();
    vertices.shutdown();
    vertexCount = 0;
}

void VertexGeometryData::setVertex(const Vertex *vertex, const unsigned int index) {
    FF_Memory::ff_copy(vertices[index], vertex, vertexSize);
}

void IndexGeometryData::shutdown() {
    allocation.shutdown();
    indicies.shutdown();
    indexCount = 0;
}

void IndexGeometryData::setIndex(const unsigned long value, const unsigned int index) {

    switch (indexSize) {
        case 8: {
            *static_cast<unsigned long *>(indicies[index]) = value;
            break;
        }
        case 4: {
            *static_cast<unsigned int *>(indicies[index]) = value;
            break;
        }
        case 2: {
            *static_cast<unsigned short *>(indicies[index]) = value;
            break;
        }
        case 1: {
            *static_cast<unsigned char *>(indicies[index]) = value;
            break;
        }
        default: {
            Logger::logError("Index size does not match a recognized size: " + std::to_string(indexSize));
            break;
        }
    }
}

void GeometryUtils::reassignIndex(unsigned int indexCount, unsigned int *indices, unsigned int from, unsigned int to) {
    for (unsigned int i = 0; i < indexCount; i++) {
        if (indices[i] == from) {
            indices[i] = to;
        } else if (indices[i] > from) {
            indices[i]--;
        }
    }
}

void GeometryUtils::generateNormals(unsigned int vertexCount, Vertex3d *vertices, const unsigned int indexCount, const unsigned int *indices) {
    for (unsigned int i = 0; i < indexCount; i += 3) {
        const unsigned int i0 = indices[i + 0];
        const unsigned int i1 = indices[i + 1];
        const unsigned int i2 = indices[i + 2];

        const Vector3f edge1 = vertices[i1].position - vertices[i0].position;
        const Vector3f edge2 = vertices[i2].position - vertices[i0].position;

        Vector3f normal = getVectorCrossProduct(edge1, edge2);
        normalize(normal);

        vertices[i0].normal = normal;
        vertices[i1].normal = normal;
        vertices[i2].normal = normal;
    }
}

void GeometryUtils::generateTangents(unsigned int vertexCount, Vertex* vertices, const unsigned int indexCount, void* indices) {
    const auto vertices3D = reinterpret_cast<Vertex3d *>(vertices);
    const auto indices3D = static_cast<unsigned int*>(indices);

    for (unsigned int i = 0; i < indexCount; i += 3) {
        const unsigned int i0 = indices3D[i + 0];
        const unsigned int i1 = indices3D[i + 1];
        const unsigned int i2 = indices3D[i + 2];

        const Vector3f edge1 = vertices3D[i1].position - vertices3D[i0].position;
        const Vector3f edge2 = vertices3D[i2].position - vertices3D[i0].position;

        const float delta1U = vertices3D[i1].textureCoordinate.x - vertices3D[i0].textureCoordinate.x;
        const float delta1V = vertices3D[i1].textureCoordinate.y - vertices3D[i0].textureCoordinate.y;

        const float delta2U = vertices3D[i2].textureCoordinate.x - vertices3D[i0].textureCoordinate.x;
        const float delta2V = vertices3D[i2].textureCoordinate.y - vertices3D[i0].textureCoordinate.y;

        const float dividend = delta1U * delta2V - delta2U * delta1V;
        const float fc = 1 / dividend;

        Vector3f tangent = {
            (fc * (delta2V * edge1.x - delta1V * edge2.x)),
            (fc * (delta2V * edge1.y - delta1V * edge2.y)),
            (fc * (delta2V * edge1.z - delta1V * edge2.z))};

        normalize(tangent);

        const float sx = delta1U;
        const float sy = delta2U;
        const float tx = delta1V;
        const float ty = delta2V;
        const float handedness = ((tx * sy - ty * sx) < 0) ? -1 : 1;
        const Vector4f t4 = toVector4f(tangent, handedness);
        vertices3D[i0].tangent = t4;
        vertices3D[i1].tangent = t4;
        vertices3D[i2].tangent = t4;
    }
}

//Move this to out of engine
void GeometryUtils::filterVertices(const unsigned int vertexCount, Vertex* vertices, const unsigned int indexCount, void* indices, unsigned int &outVertexCount, DynamicArray<Vertex3d>& outVertices) {
    const auto vertices3D = reinterpret_cast<Vertex3d*>(vertices);
    const auto indices3D = static_cast<unsigned int*>(indices);

    for (unsigned int i = 0; i < vertexCount; i++) {
        Vertex3d& vertex = vertices3D[i];
        bool found = false;

        for (unsigned int j = 0; j < outVertices.getLength(); j++) {
            if (outVertices[j] == vertex) {
                reassignIndex(indexCount, indices3D, i - outVertices.getLength(), j);
                found = true;
                break;
            }
        }

        if (!found) {
            outVertices.push(vertex);
        }
    }

    outVertexCount = outVertices.getLength();
    const unsigned int removedCount = vertexCount - outVertexCount;
    Logger::logDebug("Filter removed " + std::to_string(removedCount) + " vertices. From " + std::to_string(vertexCount) + " to " + std::to_string(outVertexCount));
}

void GeometryUtils::destroyConfig(GeometryConfig *config) {
    if (!config) return;

    config->vertices.shutdown();
    config->indices.shutdown();

    config->materialName.clear();
    config->materialPath.clear();
    config->name.clear();
}
