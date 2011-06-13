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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

struct XModelToken
{
    const uint8_t *ptr;
    size_t size;
};

enum XModelParseState
{
    kNone,
    kHeaderDeclation,
    kMeshVerticesDeclation,
    kMeshVertices,
    kMeshVertexFacesSize,
    kMeshVertexFaces,
    kMeshNormalsDeclation,
    kMeshNormals,
    kMeshNormalFacesSize,
    kMeshNormalFaces,
    kMeshMaterialListDeclation,
    kMeshMaterialFaceIndicesSize,
    kMeshMaterialFaceIndices,
    kMeshMaterialDeclation,
    kMeshMaterialPower,
    kMeshMaterialSpecularColor,
    kMeshMaterialEmmisiveColor,
    kMeshTextureFilenameDeclation,
    kMeshMaterialTextureFilename,
    kMeshTextureCoordsDeclation,
    kMeshTextureCoords,
    kMeshVertexColorsDeclation,
    kMeshVertexColors
};

XModel::XModel(const uint8_t *data, size_t size)
    : m_data(data),
      m_size(size),
      m_buffer(0)
{
}

XModel::~XModel()
{
    m_data = 0;
    delete m_buffer;
}

bool XModel::preparse()
{
    uint8_t *ptr = const_cast<uint8_t *>(m_data);
    if (16 > m_size)
        return false;

    if (!internal::stringEquals(ptr, reinterpret_cast<const uint8_t *>("xof"), 3))
        return false;

    if (!internal::stringEquals(ptr + 8, reinterpret_cast<const uint8_t *>("txt"), 3))
        return false;

    m_buffer = new char[m_size + 1];
    memcpy(m_buffer, m_data, m_size);
    m_buffer[m_size] = 0;

    return true;
}

bool XModel::load()
{
    btAlignedObjectArray<char *> tokens;
    char *p = 0, *token = 0;
    bool skip = false;
    internal::stringToken(m_buffer, " \r\n\t", &p); // signature
    internal::stringToken(NULL, " \r\n\t", &p); // version
    internal::stringToken(NULL, " \r\n\t", &p); // size
    while ((token = internal::stringToken(NULL, " \r\n\t", &p))) {
        if (skip && *token == '}') {
            skip = false;
            continue;
        }
        else if (!internal::stringEquals(token, "template", 8)) {
            skip = true;
        }
        if (skip)
            continue;
        tokens.push_back(token);
    }

    btVector4 color, specular, emmisive;
    float power = 0;

    int evsize = 0, ensize = 0, evfsize = 0, enfsize = 0, emsize = 0, efisize = 0, etsize = 0, ecsize = 0;
    int ntokens = tokens.size(), depth = 0, mindex = -1, nindices = 0;
    btAlignedObjectArray<uint32_t> indices;
    XModelParseState state = kNone;
    for (int i = 0; i < ntokens; i++) {
        char *token = tokens[i];
        if (*token == '{') {
            switch (state) {
            case kHeaderDeclation:
            case kMeshVerticesDeclation:
            case kMeshNormalsDeclation:
            case kMeshMaterialListDeclation:
            case kMeshMaterialDeclation:
            case kMeshTextureFilenameDeclation:
            case kMeshTextureCoordsDeclation:
            case kMeshVertexColorsDeclation:
                break;
            default:
                return false;
            }
        }
        else if (*token == '}') {
            state = kNone;
            depth--;
        }
        else {
            switch (state) {
            case kNone:
            {
                if (depth == 0 && !internal::stringEquals(token, "Header", 6))
                    state = kHeaderDeclation;
                else if (depth == 0 && !internal::stringEquals(token, "Mesh", 4))
                    state = kMeshVerticesDeclation;
                else if (depth == 0 && !internal::stringEquals(token, "MeshNormals", 11))
                    state = kMeshNormalsDeclation;
                else if (depth == 1 && !internal::stringEquals(token, "MeshMaterialList", 16))
                    state = kMeshMaterialListDeclation;
                else if (depth == 2 && !internal::stringEquals(token, "Material", 8))
                    state = kMeshMaterialDeclation;
                else if (depth == 3 && !internal::stringEquals(token, "TextureFilename", 15))
                    state = kMeshTextureFilenameDeclation;
                else if (depth == 1 && !internal::stringEquals(token, "MeshTextureCoords", 17))
                    state = kMeshTextureCoordsDeclation;
                else if (depth == 1 && !internal::stringEquals(token, "MeshVertexColors", 16))
                    state = kMeshVertexColorsDeclation;
                else
                    return false;
                depth++;
                break;
            }
            case kHeaderDeclation:
            {
                continue;
            }
            case kMeshVerticesDeclation:
            {
                evsize = internal::stringToInt(token);
                state = kMeshVertices;
                break;
            }
            case kMeshVertices:
            {
                char *x = internal::stringToken(token, ";", &p);
                char *y = internal::stringToken(NULL, ";", &p);
                char *z = internal::stringToken(NULL, ";", &p);
                if (x && y && z) {
                    btVector3 v(internal::stringToFloat(x), internal::stringToFloat(y),
                                internal::stringToFloat(z));
                    m_vertices.push_back(v);
                }
                else {
                    return false;
                }
                if (evsize <= m_vertices.size())
                    state = kMeshVertexFacesSize;
                break;
            }
            case kMeshVertexFacesSize:
            {
                evfsize = internal::stringToInt(token);
                state = kMeshVertexFaces;
                nindices = 0;
                indices.clear();
                break;
            }
            case kMeshVertexFaces:
            {
                char *s = internal::stringToken(token, ";", &p);
                char *x = internal::stringToken(NULL, ",", &p);
                char *y = internal::stringToken(NULL, ",", &p);
                char *z = internal::stringToken(NULL, ",", &p);
                char *w = internal::stringToken(NULL, ",", &p);
                if (s) {
                    int size = internal::stringToInt(s);
                    if (size == 3 && x && y && z) {
                        indices.push_back(internal::stringToInt(x));
                        indices.push_back(internal::stringToInt(y));
                        indices.push_back(internal::stringToInt(z));
                        nindices++;
                    }
                    else if (size == 4 && x && y && z && w) {
                        indices.push_back(internal::stringToInt(x));
                        indices.push_back(internal::stringToInt(y));
                        indices.push_back(internal::stringToInt(z));
                        indices.push_back(internal::stringToInt(z));
                        indices.push_back(internal::stringToInt(w));
                        indices.push_back(internal::stringToInt(x));
                        nindices++;
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return false;
                }
                if (evfsize <= nindices)
                    state = kNone;
                break;
            }
            case kMeshNormalsDeclation:
            {
                ensize = internal::stringToInt(token);
                state = kMeshNormals;
                break;
            }
            case kMeshNormals:
            {
                char *x = internal::stringToken(token, ";", &p);
                char *y = internal::stringToken(NULL, ";", &p);
                char *z = internal::stringToken(NULL, ";", &p);
                if (x && y && z) {
                    btVector3 v(internal::stringToFloat(x), internal::stringToFloat(y),
                                internal::stringToFloat(z));
                    m_normals.push_back(v);
                }
                else {
                    return false;
                }
                if (ensize <= m_normals.size())
                    state = kMeshNormalFacesSize;
                break;
            }
            case kMeshNormalFacesSize:
            {
                enfsize = internal::stringToInt(token);
                state = kMeshNormalFaces;
                nindices = 0;
                indices.clear();
                break;
            }
            case kMeshNormalFaces:
            {
                char *s = internal::stringToken(token, ";", &p);
                char *x = internal::stringToken(NULL, ",", &p);
                char *y = internal::stringToken(NULL, ",", &p);
                char *z = internal::stringToken(NULL, ",", &p);
                char *w = internal::stringToken(NULL, ",", &p);
                if (s) {
                    int size = internal::stringToInt(s);
                    if (size == 3 && x && y && z) {
                        indices.push_back(internal::stringToInt(x));
                        indices.push_back(internal::stringToInt(y));
                        indices.push_back(internal::stringToInt(z));
                        nindices++;
                    }
                    else if (size == 4 && x && y && z && w) {
                        indices.push_back(internal::stringToInt(x));
                        indices.push_back(internal::stringToInt(y));
                        indices.push_back(internal::stringToInt(z));
                        indices.push_back(internal::stringToInt(z));
                        indices.push_back(internal::stringToInt(w));
                        indices.push_back(internal::stringToInt(x));
                        nindices++;
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return false;
                }
                if (enfsize <= nindices)
                    state = kNone;
                break;
            }
            case kMeshMaterialListDeclation:
            {
                emsize = internal::stringToInt(token);
                state = kMeshMaterialFaceIndicesSize;
                break;
            }
            case kMeshMaterialFaceIndicesSize:
            {
                efisize = internal::stringToInt(token);
                state = kMeshMaterialFaceIndices;
                break;
            }
            case kMeshMaterialFaceIndices:
            {
                int index = internal::stringToInt(token);
                if (index < evfsize)
                    m_indices.push_back(index);
                if (efisize <= m_indices.size())
                    state = kNone;
                break;
            }
            case kMeshMaterialDeclation:
            {
                char *x = internal::stringToken(token, ";", &p);
                char *y = internal::stringToken(NULL, ";", &p);
                char *z = internal::stringToken(NULL, ";", &p);
                char *w = internal::stringToken(NULL, ";", &p);
                if (x && y && z && w) {
                    color.setValue(internal::stringToFloat(x), internal::stringToFloat(y),
                                   internal::stringToFloat(z), internal::stringToFloat(w));
                    state = kMeshMaterialPower;
                    mindex++;
                }
                else {
                    return false;
                }
                break;
            }
            case kMeshMaterialPower:
            {
                power = internal::stringToFloat(token);
                state = kMeshMaterialSpecularColor;
                break;
            }
            case kMeshMaterialSpecularColor:
            {
                char *x = internal::stringToken(token, ";", &p);
                char *y = internal::stringToken(NULL, ";", &p);
                char *z = internal::stringToken(NULL, ";", &p);
                if (x && y && z) {
                    specular.setValue(internal::stringToFloat(x), internal::stringToFloat(y),
                                      internal::stringToFloat(z), 1.0f);
                    state = kMeshMaterialEmmisiveColor;
                }
                else {
                    return false;
                }
                break;
            }
            case kMeshMaterialEmmisiveColor:
            {
                char *x = internal::stringToken(token, ";", &p);
                char *y = internal::stringToken(NULL, ";", &p);
                char *z = internal::stringToken(NULL, ";", &p);
                if (x && y && z) {
                    emmisive.setValue(internal::stringToFloat(x), internal::stringToFloat(y),
                                      internal::stringToFloat(z), 1.0f);
                    state = kNone;
                }
                else {
                    return false;
                }
                break;
            }
            case kMeshTextureFilenameDeclation:
            {
                m_textures.insert(btHashInt(mindex), token);
                break;
            }
            case kMeshTextureCoordsDeclation:
            {
                etsize = internal::stringToInt(token);
                state = kMeshTextureCoords;
                break;
            }
            case kMeshTextureCoords:
            {
                char *x = internal::stringToken(token, ";", &p);
                char *y = internal::stringToken(NULL, ";", &p);
                if (x && y) {
                    btVector3 v(internal::stringToFloat(x), internal::stringToFloat(y), 0.0f);
                    m_coords.push_back(v);
                }
                else {
                    return false;
                }
                if (etsize <= m_coords.size())
                    state = kNone;
                break;
            }
            case kMeshVertexColorsDeclation:
            {
                ecsize = internal::stringToInt(token);
                state = kMeshVertexColors;
                break;
            }
            case kMeshVertexColors:
            {
                char *index = internal::stringToken(token, ";", &p);
                char *r = internal::stringToken(NULL, ";", &p);
                char *g = internal::stringToken(NULL, ";", &p);
                char *b = internal::stringToken(NULL, ";", &p);
                char *a = internal::stringToken(NULL, ";", &p);
                if (index && r && g && b && a) {
                    btVector4 v(internal::stringToFloat(r), internal::stringToFloat(g),
                                internal::stringToFloat(b), internal::stringToFloat(a));
                    m_colors.insert(btHashInt(internal::stringToInt(index)), v);
                }
                else {
                    return false;
                }
                if (ecsize <= m_colors.size())
                    state = kNone;
                break;
            }
            default:
            {
                return false;
            }
            }
        }
    }

    return true;
}

size_t XModel::stride(StrideType type) const
{
    switch (type) {
    case kVerticesStride:
    case kNormalsStride:
    case kTextureCoordsStride:
    case kColorsStride:
        return sizeof(btVector3);
    case kIndicesStride:
        return sizeof(uint16_t);
    default:
        return 0;
    }
}

const void *XModel::verticesPointer() const
{
    return &m_vertices[0];
}

const void *XModel::normalsPointer() const
{
    return &m_normals[0];
}

const void *XModel::textureCoordsPointer() const
{
    return &m_coords[0];
}

const void *XModel::colorsPointer() const
{
    return 0;
}

const void *XModel::indicesPointer() const
{
    return &m_indices[0];
}

}
