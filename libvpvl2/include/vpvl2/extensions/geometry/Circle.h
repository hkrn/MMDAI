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
    int segments() const { return m_segments; }
    void setRadius(const Scalar &value) { m_radius = value; }
    void setThetaStart(const Scalar &value) { m_thetaStart = value; }
    void setThetaLength(const Scalar &value) { m_thetaLength = value; }
    void setSegments(int value) { m_segments = value; }

    void create() {
        Vector3 centerUV(0.5f, 0.5f, 0);
        m_vertices.append(kZeroV3);
        m_uvs.append(centerUV);
        for (int i = 0; i <= m_segments; i++) {
            const Scalar &segment = m_thetaStart + Scalar(i) / m_segments * m_thetaLength;
            Vector3 vertex(m_radius * btCos(segment), m_radius * btSin(segment), 0);
            Vector3 uv((vertex.x() / m_radius + 1) / 2, (vertex.y() / m_radius + 1) / 2, 0);
            m_vertices.append(vertex);
            m_uvs.append(uv);
        }
        btAlignedObjectArray<Vector3> &vertexUVs = m_faceVertexUVs[0];
        for (int i = 0; i <= m_segments; i++) {
            m_face3s.append(Face3(i, i + 1, 0));
            vertexUVs.push_back(m_uvs[i]);
            vertexUVs.push_back(m_uvs[i + 1]);
            vertexUVs.push_back(centerUV);
        }
        computeCentroid();
        computeFaceNormals();
    }
    void appendToModel(IModel *model) const {
        const int nvertices = m_vertices.count();
        Array<int> indices;
        model->getIndices(indices);
        IMaterial *material = createMaterial(model);
        IMaterial::IndexRange indexRange;
        indexRange.start = indices.count();
        for (int i = 0; i < nvertices; i++) {
            const Face3 &face = m_face3s[i];
            const Vector3 &origin = m_vertices[i], &uv = m_uvs[i];
            IVertex *vertex = m_vertexPtr = model->createVertex();
            vertex->setBoneRef(0, NullBone::sharedReference());
            vertex->setOrigin(origin);
            //vertex->setNormal(face.normal);
            vertex->setTextureCoord(uv);
            vertex->setMaterial(material);
            model->addVertex(vertex);
            for (int j = 0; j < 3; j++) {
                int index = face.indices[j];
                indices.append(index);
                indexRange.count++;
            }
        }
        indexRange.end = indexRange.start + indexRange.count;
        material->setIndexRange(indexRange);
        material->setFlags(IMaterial::kDisableCulling);
        model->addMaterial(material);
        model->setIndices(indices);
        m_materialPtr = 0;
        m_vertexPtr = 0;
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
