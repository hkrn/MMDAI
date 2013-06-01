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
#ifndef VPVL2_EXTENSIONS_GEOMETRY_SPHERE_H_
#define VPVL2_EXTENSIONS_GEOMETRY_SPHERE_H_

#include <vpvl2/extensions/geometry/BaseGeometry.h>

namespace vpvl2
{
namespace extensions
{
namespace geometry
{

class Sphere : public BaseGeometry {
public:
    Sphere()
        : BaseGeometry(),
          m_radius(50),
          m_phiStart(0),
          m_phiLength(SIMD_2_PI),
          m_thetaStart(0),
          m_thetaLength(SIMD_PI),
          m_widthSegments(8),
          m_heightSegments(6)
    {
    }
    ~Sphere() {
    }

    Scalar radius() const { return m_radius; }
    Scalar phiStart() const { return m_phiStart; }
    Scalar phiLenght() const { return m_phiLength; }
    Scalar thetaStart() const { return m_thetaStart; }
    Scalar thetaLength() const { return m_thetaLength; }
    int widthSegments() const { return m_widthSegments; }
    int heightSegments() const { return m_heightSegments; }
    void setRadius(const Scalar &value) { m_radius = btMax(value, 0.0f); }
    void setPhiStart(const Scalar &value) { m_phiStart = btClamped(value, 0.0f, SIMD_2_PI); }
    void setPhiLength(const Scalar &value) { m_phiLength = btClamped(value, 0.0f, SIMD_2_PI); }
    void setThetaStart(const Scalar &value) { m_thetaStart = btClamped(value, 0.0f, SIMD_PI); }
    void setThetaLength(const Scalar &value) { m_thetaLength = btClamped(value, 0.0f, SIMD_PI); }
    void setWidthSegments(int value) { m_widthSegments = btMax(value, 3); }
    void setHeightSegments(int value) { m_heightSegments = btMax(value, 2); }

    void create() {
        IndexList rowIndices;
        UVList rowUVs;
        btAlignedObjectArray<IndexList> indices;
        btAlignedObjectArray<UVList> uvs;
        Vector3 vertex, uv;
        for (int i = 0; i <= m_heightSegments; i++) {
            rowIndices.clear();
            rowUVs.clear();
            for (int j = 0; j <= m_widthSegments; j++) {
                const Scalar &u = Scalar(j) / m_widthSegments;
                const Scalar &v = Scalar(i) / m_heightSegments;
                const Scalar &p = m_phiStart + u * m_phiLength;
                const Scalar &t = m_thetaStart + v * m_thetaLength;
                vertex.setX(-m_radius * btCos(p) * btSin(t));
                vertex.setY(m_radius * btCos(t));
                vertex.setZ(m_radius * btSin(p) * btSin(t));
                uv.setValue(u, v, 0);
                m_vertices.append(vertex);
                m_uvs.append(uv);
                rowIndices.push_back(m_vertices.count() - 1);
                rowUVs.push_back(uv);
            }
            indices.push_back(rowIndices);
            uvs.push_back(rowUVs);
        }
        for (int i = 0; i < m_heightSegments; i++) {
            for (int j = 0; j < m_widthSegments; j++) {
                int i1 = indices[i][j + 1];
                int i2 = indices[i][j];
                int i3 = indices[i + 1][j];
                int i4 = indices[i + 1][j + 1];
                if (btFabs(m_vertices[i1].y()) == m_radius) {
                    addFace(i1, i3, i4);
                }
                else if (btFabs(m_vertices[i3].y()) == m_radius) {
                    addFace(i1, i2, i3);
                }
                else {
                    addFace(i1, i2, i4);
                    addFace(i2, i3, i4);
                }
            }
        }
        computeCentroid();
        computeFaceNormals();
    }
    void appendToModel(IModel *model) const {
        IMaterial *material = createMaterial(model);
        buildMaterial(model, material);
        model->addMaterial(material);
        resetPointers();
    }

private:
    void addFace(int a, int b, int c) {
        Face3 face(a, c, b);
        NormalList &n = face.vertexNormals;
        n.push_back(m_vertices[a].normalized());
        n.push_back(m_vertices[c].normalized());
        n.push_back(m_vertices[b].normalized());
        m_faces.append(face);
        UVList &uvs = m_faceVertexUVs[0];
        uvs.push_back(m_uvs[a]);
        uvs.push_back(m_uvs[c]);
        uvs.push_back(m_uvs[b]);
    }

    Scalar m_radius;
    Scalar m_phiStart;
    Scalar m_phiLength;
    Scalar m_thetaStart;
    Scalar m_thetaLength;
    int m_widthSegments;
    int m_heightSegments;
};

} /* namespace geometry */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
