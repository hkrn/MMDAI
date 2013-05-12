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
#ifndef VPVL2_EXTENSIONS_GEOMETRY_TORUS_H_
#define VPVL2_EXTENSIONS_GEOMETRY_TORUS_H_

#include <vpvl2/extensions/geometry/BaseGeometry.h>

namespace vpvl2
{
namespace extensions
{
namespace geometry
{

class Torus : public BaseGeometry {
public:
    Torus()
        : BaseGeometry(),
          m_radius(100),
          m_tube(40),
          m_arc(SIMD_2_PI),
          m_radialSegments(8),
          m_tubularSegments(6)
    {
    }
    ~Torus() {
    }

    Scalar radius() const { return m_radius; }
    Scalar tube() const { return m_tube; }
    Scalar arc() const { return m_arc; }
    int radialSegments() const { return m_radialSegments; }
    int tubularSegments() const { return m_tubularSegments; }
    void setRadius(const Scalar &value) { m_radius = btMax(value, 0.0f); }
    void setTube(const Scalar &value) { m_tube = btMax(value, 0.0f); }
    void setArc(const Scalar &value) { m_arc = btClamped(value, 0.0f, SIMD_2_PI); }
    void setRadialSegments(int value) { m_radialSegments = btMax(value, 1); }
    void setTubularSegments(int value) { m_tubularSegments = btMax(value, 1); }

    void create() {
        Vector3 center(kZeroV3), vertex(kZeroV3), uv(kZeroV3);
        NormalList normals;
        UVList uvs;
        for (int i = 0; i <= m_radialSegments; i++) {
            for (int j = 0; j <= m_tubularSegments; j++) {
                const Scalar &u = Scalar(j) / m_tubularSegments * m_arc;
                const Scalar &v = Scalar(i) / m_radialSegments * SIMD_2_PI;
                const Scalar &t = m_radius + m_tube * btCos(v);
                center.setValue(m_radius * btCos(u), m_radius * btSin(u), 0);
                vertex.setX(t * btCos(u));
                vertex.setY(t * btSin(u));
                vertex.setZ(m_tube * btSin(v));
                m_vertices.append(vertex);
                uv.setValue(Scalar(j) / m_tubularSegments, 1 - Scalar(i) / m_radialSegments, 0);
                uvs.push_back(uv);
                normals.push_back((vertex - center).normalized());
            }
        }
        UVList &faceVertexUVs = m_faceVertexUVs[0];
        for (int i = 1; i <= m_radialSegments; i++) {
            for (int j = 1; j <= m_tubularSegments; j++) {
                int i4 = (m_tubularSegments + 1) * i + j, i1 = i4 - 1;
                int i3 = (m_tubularSegments + 1) * (i - 1) + j, i2 = i3 - 1;
                Face3 face(i1, i3, i2), face2(i1, i4, i3);
                face.vertexNormals.push_back(normals[i1]);
                face.vertexNormals.push_back(normals[i3]);
                face.vertexNormals.push_back(normals[i2]);
                face2.vertexNormals.push_back(normals[i1]);
                face2.vertexNormals.push_back(normals[i4]);
                face2.vertexNormals.push_back(normals[i3]);
                /* TODO: normalize normal array of the face */
                m_faces.append(face);
                m_faces.append(face2);
                faceVertexUVs.push_back(uvs[i1]);
                faceVertexUVs.push_back(uvs[i3]);
                faceVertexUVs.push_back(uvs[i2]);
                faceVertexUVs.push_back(uvs[i1]);
                faceVertexUVs.push_back(uvs[i4]);
                faceVertexUVs.push_back(uvs[i3]);
            }
        }
        computeCentroid();
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
    Scalar m_tube;
    Scalar m_arc;
    int m_radialSegments;
    int m_tubularSegments;
};

} /* namespace geometry */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
