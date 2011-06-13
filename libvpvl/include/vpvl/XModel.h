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

#include "vpvl/PMDModel.h"
#include "vpvl/common.h"

namespace vpvl
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * XModel class represents a DirectX model.
 */

class XModel
{
public:
    enum StrideType
    {
        kVerticesStride,
        kNormalsStride,
        kTextureCoordsStride,
        kColorsStride,
        kIndicesStride
    };

    XModel(const uint8_t *data, size_t size);
    ~XModel();

    bool preparse();
    bool load();

    size_t stride(StrideType type) const;
    const void *verticesPointer() const;
    const void *normalsPointer() const;
    const void *textureCoordsPointer() const;
    const void *colorsPointer() const;
    const void *indicesPointer() const;

    const btAlignedObjectArray<btVector3> &vertices() const {
        return m_vertices;
    }
    const btAlignedObjectArray<btVector3> &normals() const {
        return m_normals;
    }
    const btAlignedObjectArray<btVector3> &textureCoords() const {
        return m_coords;
    }
    const char *findTexture(int index) const {
        const char **ptr = const_cast<const char **>(m_textures.find(btHashInt(index)));
        return ptr ? *ptr : 0;
    }

private:
    btAlignedObjectArray<btVector3> m_vertices;
    btAlignedObjectArray<btVector3> m_normals;
    btAlignedObjectArray<btVector3> m_coords;
    btAlignedObjectArray<uint16_t> m_indices;
    btHashMap<btHashInt, btVector4> m_colors;
    btHashMap<btHashInt, char *> m_textures;
    const uint8_t *m_data;
    const size_t m_size;
    char *m_buffer;
};

}

#endif
