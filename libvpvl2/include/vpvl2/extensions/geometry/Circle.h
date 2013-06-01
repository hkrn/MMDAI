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
#ifndef VPVL2_EXTENSIONS_GEOMETRY_CIRCLE_H_
#define VPVL2_EXTENSIONS_GEOMETRY_CIRCLE_H_

#include <vpvl2/extensions/geometry/BaseGeometry.h>

namespace vpvl2
{
namespace extensions
{
namespace geometry
{

class Circle : public BaseGeometry {
public:
    Circle()
        : BaseGeometry(),
          m_radius(50),
          m_thetaStart(0),
          m_thetaLength(SIMD_2_PI),
          m_segments(8)
    {
    }
    ~Circle() {
    }

    Scalar radius() const { return m_radius; }
    Scalar thetaStart() const { return m_thetaStart; }
    Scalar thetaLength() const { return m_thetaLength; }
    int segments() const { return btMax(m_segments, 1); }
    void setRadius(const Scalar &value) { m_radius = btMax(value, 0.0f); }
    void setThetaStart(const Scalar &value) { m_thetaStart = btClamped(value, 0.0f, SIMD_2_PI); }
    void setThetaLength(const Scalar &value) { m_thetaLength = btClamped(value, 0.0f, SIMD_2_PI); }
    void setSegments(int value) { m_segments = btMax(value, 1); }

    void create() {
        Vector3 centerUV(0.5f, 0.5f, 0), vertex(kZeroV3), uv(kZeroV3);
        m_vertices.append(kZeroV3);
        m_uvs.append(centerUV);
        for (int i = 0; i <= m_segments; i++) {
            const Scalar &segment = m_thetaStart + Scalar(i) / m_segments * m_thetaLength;
            vertex.setValue(m_radius * btCos(segment), m_radius * btSin(segment), 0);
            uv.setValue(vertex.x() / m_radius + 1, -vertex.y() / m_radius + 1, 0);
            m_vertices.append(vertex);
            m_uvs.append(uv * 0.5f);
        }
        UVList &vertexUVs = m_faceVertexUVs[0];
        const Vector3 normal(0, 0, 1);
        for (int i = 1; i <= m_segments; i++) {
            Face3 face(i, i + 1, 0);
            face.setNormal(normal);
            m_faces.append(face);
            vertexUVs.push_back(m_uvs[i]);
            vertexUVs.push_back(m_uvs[i + 1]);
            vertexUVs.push_back(centerUV);
        }
        computeCentroid();
        computeFaceNormals();
    }
    void appendToModel(IModel *model) const {
        IMaterial *material = createMaterial(model);
        buildMaterial(model, material);
        material->setFlags(IMaterial::kDisableCulling);
        model->addMaterial(material);
        resetPointers();
    }

private:
    Scalar m_radius;
    Scalar m_thetaStart;
    Scalar m_thetaLength;
    int m_segments;
};

} /* namespace geometry */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
