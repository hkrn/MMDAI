/**

The MIT License

Copyright (c) 2010-2013 three.js authors
              2013 hkrn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

 */

#pragma once
#ifndef VPVL2_EXTENSIONS_GEOMETRY_CYLINDER_H_
#define VPVL2_EXTENSIONS_GEOMETRY_CYLINDER_H_

#include <vpvl2/extensions/geometry/BaseGeometry.h>

namespace vpvl2
{
namespace extensions
{
namespace geometry
{

class Cylinder : public BaseGeometry {
public:
    Cylinder()
        : BaseGeometry(),
          m_radiusTop(20),
          m_radiusBottom(20),
          m_height(100),
          m_radiusSegments(8),
          m_heightSegments(1),
          m_openEnded(false)
    {
    }
    ~Cylinder() {
    }

    Scalar radiusTop() const { return m_radiusTop; }
    Scalar radiusBottom() const { return m_radiusBottom; }
    Scalar height() const { return m_height; }
    int radiusSegments() const { return m_radiusSegments; }
    int heightSegments() const { return m_heightSegments; }
    bool openEnded() const { return m_openEnded; }
    void setRadiusTop(const Scalar &value) { m_radiusTop = value; }
    void setRadiusBottom(const Scalar &value) { m_radiusBottom = value; }
    void height(const Scalar &value) { m_height = value; }
    void setRadiusSegments(int value) { m_radiusSegments = value; }
    void setHeightSegments(int value) { m_heightSegments = value; }
    void setOpenEnded(bool value) { m_openEnded = value; }

    void create() {
        const Scalar &heightHalf = m_height * 0.5f;
        IndexList rowIndices;
        UVList rowUVs;
        btAlignedObjectArray<IndexList> indices;
        btAlignedObjectArray<UVList> uvs;
        Vector3 vertex, uv;
        for (int i = 0; i <= m_heightSegments; i++) {
            const Scalar &v = Scalar(i) / m_heightSegments;
            const Scalar &radius = v * (m_radiusBottom - m_radiusTop) + m_radiusTop;
            rowIndices.clear();
            rowUVs.clear();
            for (int j = 0; j <= m_radiusSegments; j++) {
                const Scalar &u = Scalar(j) / m_radiusSegments;
                vertex.setX(radius * btSin(u * SIMD_2_PI));
                vertex.setY(-v * m_height + heightHalf);
                vertex.setZ(radius * btCos(u * SIMD_2_PI));
                uv.setValue(u, v, 0); /* three.js does it to (u, 1 - v) */
                m_vertices.append(vertex);
                m_uvs.append(uv);
                rowIndices.push_back(m_vertices.count() - 1);
                rowUVs.push_back(uv);
            }
            indices.push_back(rowIndices);
            uvs.push_back(rowUVs);
        }
        const Scalar &tanTheta = (m_radiusBottom - m_radiusTop) / m_height;
        UVList &vertexUVs = m_faceVertexUVs[0];
        Vector3 na, nb;
        for (int i = 0; i < m_radiusSegments; i++) {
            if (m_radiusTop != 0) {
                const IndexList &rowIndices = indices[0];
                na = m_vertices[rowIndices[i]];
                nb = m_vertices[rowIndices[i + 1]];
            }
            else {
                const IndexList &indexRow = indices[1];
                na = m_vertices[indexRow[i]];
                nb = m_vertices[indexRow[i + 1]];
            }
            na.setY(btSqrt(na.x() * na.x() + na.z() + na.z()) * tanTheta);
            na.normalize();
            nb.setY(btSqrt(nb.x() * nb.x() + nb.z() + nb.z()) * tanTheta);
            nb.normalize();
            for (int j = 0; j < m_heightSegments; j++) {
                int i2 = indices[j + 1][i], i4 = indices[j][i + 1];
                Face3 face1(indices[j][i], i4, i2), face2(i2, i4, indices[j + 1][i + 1]);
                NormalList &n1 = face1.vertexNormals, &n2 = face2.vertexNormals;
                n1.push_back(na);
                n1.push_back(na);
                n1.push_back(nb);
                n2.push_back(na);
                n2.push_back(nb);
                n2.push_back(nb);
                m_face3s.append(face1);
                m_face3s.append(face2);
                const Vector3 &uv2 = uvs[j + 1][i], &uv4 = uvs[j][i + 1];
                vertexUVs.push_back(uvs[j][i]);
                vertexUVs.push_back(uv4);
                vertexUVs.push_back(uv2);
                vertexUVs.push_back(uv2);
                vertexUVs.push_back(uv4);
                vertexUVs.push_back(uvs[j + 1][i + 1]);
            }
        }
        if (!m_openEnded && m_radiusTop > 0) {
            m_vertices.append(Vector3(0, heightHalf, 0));
            const int lastIndex = m_vertices.count() - 1;
            const IndexList &rowIndices = indices[0];
            const UVList &rowUVs = uvs[0];
            for (int i = 0; i < m_radiusSegments; i++) {
                int i1 = rowIndices[i];
                int i2 = rowIndices[i + 1];
                Face3 face(i1, lastIndex, i2);
                face.setNormal(Vector3(0, 1, 0));
                m_face3s.append(face);
                const Vector3 &uv2 = rowUVs[i + 1];
                vertexUVs.push_back(rowUVs[i]);
                vertexUVs.push_back(Vector3(uv2.x(), 1, 0));
                vertexUVs.push_back(uv2);
            }
        }
        if (!m_openEnded && m_radiusBottom > 0) {
            m_vertices.append(Vector3(0, -heightHalf, 0));
            const int lastIndex = m_vertices.count() - 1;
            const IndexList &rowIndices = indices[m_heightSegments];
            const UVList &rowUVs = uvs[m_heightSegments];
            for (int i = 0; i < m_radiusSegments; i++) {
                int i1 = rowIndices[i + 1];
                int i2 = rowIndices[i];
                Face3 face(i1, lastIndex, i2);
                face.setNormal(Vector3(0, -1, 0));
                m_face3s.append(face);
                const Vector3 &uv2 = rowUVs[i];
                vertexUVs.push_back(rowUVs[i + 1]);
                vertexUVs.push_back(Vector3(uv2.x(), 0, 0));
                vertexUVs.push_back(uv2);
            }
        }
        computeCentroid();
        computeFaceNormals();
    }
    void appendToModel(IModel *model) const {
        IMaterial *material = createMaterial(model);
        buildMaterial(model, material);
        if (m_openEnded) {
            material->setFlags(IMaterial::kDisableCulling);
        }
        model->addMaterial(material);
        resetPointers();
    }

private:
    Scalar m_radiusTop;
    Scalar m_radiusBottom;
    Scalar m_height;
    int m_radiusSegments;
    int m_heightSegments;
    bool m_openEnded;
};

} /* namespace geometry */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
