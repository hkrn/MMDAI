/**

The MIT License

Copyright (c) 2010-2014 three.js authors
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
#ifndef VPVL2_EXTENSIONS_GEOMETRY_RING_H_
#define VPVL2_EXTENSIONS_GEOMETRY_RING_H_

#include <vpvl2/extensions/geometry/BaseGeometry.h>

namespace vpvl2
{
namespace extensions
{
namespace geometry
{

class Ring : public BaseGeometry {
public:
    Ring()
        : BaseGeometry(),
          m_innerRadius(0),
          m_outerRadius(50),
          m_thetaStart(0),
          m_thetaLength(SIMD_2_PI),
          m_thetaSegments(8),
          m_phiSegments(8)
    {
    }
    ~Ring() {
    }

    Scalar innerRadius() const { return m_innerRadius; }
    Scalar outerRadius() const { return m_outerRadius; }
    Scalar thetaStart() const { return m_thetaStart; }
    Scalar thetaLength() const { return m_thetaLength; }
    int thetaSegments() const { return m_thetaSegments; }
    int phiSegments() const { return m_phiSegments; }
    void setInnerRadius(const Scalar &value) { m_innerRadius = value; }
    void setOuterRadius(const Scalar &value) { m_outerRadius = value; }
    void setThetaStart(const Scalar &value) { m_thetaStart = value; }
    void setThetaLength(const Scalar &value) { m_thetaLength = value; }
    void setThetaSegments(int value) { m_thetaSegments = btMax(value, 3); }
    void setPhiSegments(int value) { m_phiSegments = btMax(value, 3); }

    void create() {
        Scalar radius = m_innerRadius, step = ((m_outerRadius - radius) / m_phiSegments);
        UVList uvs;
        Vector3 vertex(kZeroV3), uv(kZeroV3), normal(0, 0, 1);
        for (int i = 0; i <= m_phiSegments; i++) {
            for (int j = 0; j <= m_thetaSegments; j++) {
                const Scalar &segment = m_thetaStart + Scalar(j) / m_thetaSegments * m_thetaLength;
                const Scalar &theta = btCos(segment), &phi = btSin(segment);
                vertex.setValue(radius * theta, radius * phi, 0);
                uv.setValue((theta + 1) * 0.5, (phi + 1) * 0.5, 0);
                m_vertices.append(vertex);
                uvs.push_back(uv);
            }
            radius += step;
        }
        UVList &faceVertexUVs = m_faceVertexUVs[0];
        for (int i = 0; i < m_phiSegments; i++) {
            const int thetaSegment = i * m_thetaSegments;
            for (int j = 0; j <= m_thetaSegments; j++) {
                int segment = j + thetaSegment;
                int i1 = segment + i;
                int i2 = i1 + m_thetaSegments;
                int i3 = i2 + 1;
                int i4 = i1;
                int i5 = i2 + 1;
                int i6 = i4 + 1;
                Face3 face(i1, i3, i2), face2(i4, i6, i5);
                face.setNormal(normal);
                face2.setNormal(normal);
                m_faces.append(face);
                m_faces.append(face2);
                faceVertexUVs.push_back(uvs[i1]);
                faceVertexUVs.push_back(uvs[i3]);
                faceVertexUVs.push_back(uvs[i2]);
                faceVertexUVs.push_back(uvs[i4]);
                faceVertexUVs.push_back(uvs[i6]);
                faceVertexUVs.push_back(uvs[i5]);
            }
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
    Scalar m_innerRadius;
    Scalar m_outerRadius;
    Scalar m_thetaStart;
    Scalar m_thetaLength;
    int m_thetaSegments;
    int m_phiSegments;
};

} /* namespace geometry */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
