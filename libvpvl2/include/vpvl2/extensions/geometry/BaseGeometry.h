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
#ifndef VPVL2_EXTENSIONS_GEOMETRY_BASEGEOMETRY_H_
#define VPVL2_EXTENSIONS_GEOMETRY_BASEGEOMETRY_H_

#include <vpvl2/Common.h>
#include <vpvl2/IBone.h>
#include <vpvl2/IModel.h>
#include <vpvl2/IMaterial.h>
#include <vpvl2/IVertex.h>

namespace vpvl2
{
namespace extensions
{
namespace geometry
{

class BaseGeometry {
public:
    typedef btAlignedObjectArray<int> IndexList;
    typedef btAlignedObjectArray<Vector3> NormalList;
    typedef btAlignedObjectArray<Vector3> UVList;
    typedef btAlignedObjectArray<Color> ColorList;
    struct Face3 {
        Face3(int a, int b, int c)
            : centroid(kZeroV3),
              normal(kZeroV3),
              color(kZeroC)
        {
            indices[0] = a;
            indices[1] = b;
            indices[2] = c;
        }
        int indices[3];
        Vector3 centroid;
        Vector3 normal;
        Color color;
        NormalList vertexNormals;
        ColorList vertexColors;
    };

    BaseGeometry()
        : m_vertexPtr(0),
          m_materialPtr(0),
          m_ambient(kZeroC),
          m_diffuse(kZeroC),
          m_specular(kZeroC),
          m_shininess(0)
    {
        m_faceVertexUVs.append(UVList());
    }
    virtual ~BaseGeometry() {
        delete m_materialPtr;
        m_materialPtr = 0;
        delete m_vertexPtr;
        m_vertexPtr = 0;
    }

    const Array<Vector3> &vertices() const { return m_vertices; }
    const Array<Vector3> &normals() const { return m_normals; }
    const Array<Vector3> &uvs() const { return m_uvs; }
    const Array<Face3> &face3s() const { return m_face3s; }
    Color ambient() const { return m_ambient; }
    Color diffuse() const { return m_diffuse; }
    Color specular() const { return m_specular; }
    Scalar shininess() const { return m_shininess; }
    void setAmbient(const Color &value) { m_ambient = value; }
    void setDiffuse(const Color &value) { m_diffuse = value; }
    void setSpecular(const Color &value) { m_specular = value; }
    void setShininess(const Scalar &value) { m_shininess = value; }

    void computeFaceNormals() {
        const int nfaces = m_face3s.count();
        for (int i = 0; i < nfaces; i++) {
            Face3 &face = m_face3s[i];
            const Vector3 &a = m_vertices[face.indices[0]];
            const Vector3 &b = m_vertices[face.indices[1]];
            const Vector3 &c = m_vertices[face.indices[2]];
            const Vector3 &normalized = (c - b).cross(a - b).normalized();
            face.normal = normalized;
        }
    }
    void computeCentroid() {
        const int nfaces = m_face3s.count();
        Vector3 sum(kZeroV3);
        for (int i = 0; i < nfaces; i++) {
            Face3 &face = m_face3s[i];
            sum.setZero();
            for (int j = 0; j < 3; j++) {
                sum += m_vertices[face.indices[j]];
            }
            face.centroid = sum / 3;
        }
    }

    virtual void create() = 0;
    virtual void appendToModel(IModel *model) const = 0;

protected:
    IMaterial *createMaterial(IModel *model) const {
        IMaterial *material = m_materialPtr = model->createMaterial();
        material->setAmbient(m_ambient);
        material->setDiffuse(m_diffuse);
        material->setSpecular(m_specular);
        material->setShininess(m_shininess);
        return material;
    }
    void addVerticesFromFace3(IModel *model, IMaterial *material) const {
        Array<int> indices;
        model->getIndices(indices);
        IMaterial::IndexRange indexRange;
        indexRange.start = indices.count();
        const int nfaces = m_face3s.count();
        for (int i = 0; i < nfaces; i++) {
            const Face3 &face = m_face3s[i];
            addVertex(model, material, face, i);
            for (int j = 0; j < 3; j++) {
                int index = face.indices[j];
                indices.append(index);
                indexRange.count++;
            }
        }
        indexRange.end = indexRange.start + indexRange.count;
        material->setIndexRange(indexRange);
        model->setIndices(indices);
    }
    void addVertex(IModel *model, IMaterial *material, const Face3 &face, int index) const {
        const Vector3 &origin = m_vertices[index], &uv = m_uvs[index];
        IVertex *vertex = m_vertexPtr = model->createVertex();
        vertex->setBoneRef(0, NullBone::sharedReference());
        vertex->setOrigin(origin);
        vertex->setNormal(face.normal);
        vertex->setTextureCoord(uv);
        vertex->setMaterial(material);
        model->addVertex(vertex);
    }
    void resetPointers() const {
        m_materialPtr = 0;
        m_vertexPtr = 0;
    }

    Array<Vector3> m_vertices;
    Array<Vector3> m_normals;
    Array<Vector3> m_uvs;
    Array<Face3> m_face3s;
    Array<UVList> m_faceVertexUVs;

private:
    mutable IVertex *m_vertexPtr;
    mutable IMaterial *m_materialPtr;
    Color m_ambient;
    Color m_diffuse;
    Color m_specular;
    Scalar m_shininess;
};

} /* namespace geometry */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
