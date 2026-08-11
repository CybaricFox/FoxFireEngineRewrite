//
// Created by cmorg on 8/11/2026.
//

#include "GeometryUtils.h"

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

void GeometryUtils::generateTangents(unsigned int vertexCount, Vertex3d *vertices, const unsigned int indexCount, const unsigned int *indices) {
    for (unsigned int i = 0; i < indexCount; i += 3) {
        const unsigned int i0 = indices[i + 0];
        const unsigned int i1 = indices[i + 1];
        const unsigned int i2 = indices[i + 2];

        const Vector3f edge1 = vertices[i1].position - vertices[i0].position;
        const Vector3f edge2 = vertices[i2].position - vertices[i0].position;

        const float delta1U = vertices[i1].textureCoordinate.x - vertices[i0].textureCoordinate.x;
        const float delta1V = vertices[i1].textureCoordinate.y - vertices[i0].textureCoordinate.y;

        const float delta2U = vertices[i2].textureCoordinate.x - vertices[i0].textureCoordinate.x;
        const float delta2V = vertices[i2].textureCoordinate.y - vertices[i0].textureCoordinate.y;

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
        vertices[i0].tangent = t4;
        vertices[i1].tangent = t4;
        vertices[i2].tangent = t4;
    }
}
