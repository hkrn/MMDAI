/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef VPVL_XMODEL_H_
#define VPVL_XMODEL_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btVector3.h>

#include "vpvl/PMDModel.h"
#include "vpvl/XMaterial.h"
#include "vpvl/common.h"

namespace vpvl
{

struct XModelFaceIndex {
    uint32_t index;
    uint32_t count;
    btVector4 value;
};

typedef struct XModelUserData XModelUserData;
typedef btAlignedObjectArray<uint16_t> XModelIndexList;

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * XModel class represents a DirectX model.
 */

class VPVL_EXPORT XModel
{
public:

    enum Error
    {
        kNoError,
        kInvalidHeader,
        kInvalidSignatureError,
        kNotTextFormatError,
        kDeclarationError,
        kInvalidParseStateError,
        kMeshVerticeError,
        kMeshFaceSizeError,
        kMeshFaceError,
        kMeshNormalError,
        kMeshMaterialFaceError,
        kMeshMaterialDeclarationError,
        kMeshMaterialSpecularError,
        kMeshMaterialEmmisiveError,
        kMeshTextureCoordsError,
        kMeshVertexColorError,
        kMaxErrors
    };

    enum StrideType
    {
        kVerticesStride,
        kNormalsStride,
        kTextureCoordsStride,
        kColorsStride,
        kIndicesStride
    };

    XModel();
    ~XModel();

    bool preparse(const uint8_t *data, size_t size);
    bool load(const uint8_t *data, size_t size);

    size_t stride(StrideType type) const;
    const void *verticesPointer() const;
    const void *normalsPointer() const;
    const void *textureCoordsPointer() const;
    const void *colorsPointer() const;

    const btAlignedObjectArray<btVector3> &vertices() const {
        return m_vertices;
    }
    const btAlignedObjectArray<btVector3> &normals() const {
        return m_normals;
    }
    const btAlignedObjectArray<btVector3> &textureCoords() const {
        return m_coords;
    }
    const btAlignedObjectArray<btVector4> &colors() const {
        return m_colors;
    }
    const btAlignedObjectArray<XModelFaceIndex> &faces() const {
        return m_faces;
    }
    const XModelIndexList *indicesAt(uint32_t value) const {
        return m_indices[value];
    }
    const XMaterial *materialAt(uint32_t value) const {
        return m_materials[value];
    }
    uint32_t countMatreials() const {
        return m_materials.size() - 1;
    }
    XModelUserData *userData() const {
        return m_userData;
    }

    const btVector3 &position() const {
        return m_position;
    }
    const btQuaternion &rotation() const {
        return m_rotation;
    }
    const btTransform &transform() const {
        return m_transform;
    }
    float scale() const {
        return m_scale;
    }
    float opacity() const {
        return m_opacity;
    }
    void setPosition(const btVector3 &value) {
        m_position = value;
        m_transform.setOrigin(value);
    }
    void setRotation(const btQuaternion &value) {
        m_rotation = value;
        m_transform.setRotation(value);
    }
    void setScale(float value) {
        m_scale = value;
    }
    void setOpacity(float value) {
        m_opacity = value;
    }
    void setUserData(XModelUserData *value) {
        m_userData = value;
    }

private:
    void release();

    btVector3 m_position;
    btQuaternion m_rotation;
    btTransform m_transform;
    float m_scale;
    float m_opacity;
    btAlignedObjectArray<btVector3> m_vertices;
    btAlignedObjectArray<btVector3> m_normals;
    btAlignedObjectArray<btVector3> m_coords;
    btAlignedObjectArray<btVector4> m_colors;
    btAlignedObjectArray<XModelFaceIndex> m_faces;
    btAlignedObjectArray<XModelIndexList *> m_indices;
    btAlignedObjectArray<XMaterial *> m_materials;
    XModelUserData *m_userData;
    Error m_error;

    VPVL_DISABLE_COPY_AND_ASSIGN(XModel)
};

}

#endif
