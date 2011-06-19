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

struct XModelInternalToken
{
    const uint8_t *ptr;
    size_t size;
};

struct XModelInternalIndexedColor {
    uint32_t index;
    btVector4 color;
};

struct XModelInternalMaterial {
    uint32_t index;
    btVector4 color;
    btVector4 specular;
    btVector4 emmisive;
    char *textureName;
    float power;
};

enum XModelInternalParseState
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
      m_size(size)
{
}

XModel::~XModel()
{
    internal::clearAll(m_indices);
    internal::clearAll(m_materials);
    m_vertices.clear();
    m_normals.clear();
    m_coords.clear();
    m_colors.clear();
    m_materials.clear();
    m_userData = 0;
    m_data = 0;
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

    return true;
}

bool XModel::load()
{
    if (!preparse())
        return false;

    char *buffer = new char[m_size + 1];
    memcpy(buffer, m_data, m_size);
    buffer[m_size] = 0;

    btAlignedObjectArray<char *> tokens;
    char *p = 0, *token = 0;
    bool skip = false;
    internal::stringToken(buffer, " \r\n\t", &p); // signature
    internal::stringToken(NULL, " \r\n\t", &p); // version
    internal::stringToken(NULL, " \r\n\t", &p); // size
    while ((token = internal::stringToken(NULL, " \r\n\t", &p))) {
        if (skip && *token == '}') {
            skip = false;
            continue;
        }
        else if (internal::stringEquals(token, "template", 8)) {
            skip = true;
        }
        if (skip)
            continue;
        tokens.push_back(token);
    }

    bool ret = false;
    uint32_t evsize = 0, ensize = 0, evfsize = 0, enfsize = 0, emsize = 0, efisize = 0, etsize = 0, ecsize = 0;
    uint32_t nTokens = tokens.size(), nIndices = 0, materialIndex = 0;
    int depth = 0;
    btAlignedObjectArray<XModelInternalIndexedColor> colors;
    btAlignedObjectArray<XModelInternalMaterial *> materials;
    XModelInternalMaterial *currentMaterial = 0;
    XModelInternalParseState state = kNone;

    try {
        for (uint32_t i = 0; i < nTokens; i++) {
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
                    throw 0;
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
                    if (depth == 0 && internal::stringEquals(token, "Header", 6))
                        state = kHeaderDeclation;
                    else if (depth == 0 && internal::stringEquals(token, "Mesh", 4))
                        state = kMeshVerticesDeclation;
                    else if (depth == 1 && internal::stringEquals(token, "MeshNormals", 11))
                        state = kMeshNormalsDeclation;
                    else if (depth == 1 && internal::stringEquals(token, "MeshMaterialList", 16))
                        state = kMeshMaterialListDeclation;
                    else if (depth == 2 && internal::stringEquals(token, "Material", 8))
                        state = kMeshMaterialDeclation;
                    else if (depth == 3 && internal::stringEquals(token, "TextureFilename", 15))
                        state = kMeshTextureFilenameDeclation;
                    else if (depth == 1 && internal::stringEquals(token, "MeshTextureCoords", 17))
                        state = kMeshTextureCoordsDeclation;
                    else if (depth == 1 && internal::stringEquals(token, "MeshVertexColors", 16))
                        state = kMeshVertexColorsDeclation;
                    else
                        throw 1;
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
                        throw 2;
                    }
                    if (evsize <= static_cast<uint32_t>(m_vertices.size()))
                        state = kMeshVertexFacesSize;
                    break;
                }
                case kMeshVertexFacesSize:
                {
                    evfsize = internal::stringToInt(token);
                    state = kMeshVertexFaces;
                    nIndices = 0;
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
                        uint32_t size = internal::stringToInt(s);
                        XModelFaceIndex v;
                        v.index = 0;
                        v.count = size;
                        if (size == 3 && x && y && z) {
                            v.value.setValue(static_cast<const btScalar>(internal::stringToInt(x)),
                                             static_cast<const btScalar>(internal::stringToInt(y)),
                                             static_cast<const btScalar>(internal::stringToInt(z)),
                                             0.0f);
                        }
                        else if (size == 4 && x && y && z && w) {
                            v.value.setValue(static_cast<const btScalar>(internal::stringToInt(x)),
                                             static_cast<const btScalar>(internal::stringToInt(y)),
                                             static_cast<const btScalar>(internal::stringToInt(z)),
                                             static_cast<const btScalar>(internal::stringToInt(w)));
                        }
                        else {
                            throw 3;
                        }
                        m_faces.push_back(v);
                        nIndices++;
                    }
                    else {
                        throw 4;
                    }
                    if (evfsize <= nIndices)
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
                        throw 5;
                    }
                    if (ensize <= static_cast<uint32_t>(m_normals.size()))
                        state = kMeshNormalFacesSize;
                    break;
                }
                case kMeshNormalFacesSize:
                {
                    enfsize = internal::stringToInt(token);
                    state = kMeshNormalFaces;
                    nIndices = 0;
                    break;
                }
                case kMeshNormalFaces:
                {
                    nIndices++;
                    if (enfsize <= nIndices)
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
                    nIndices = 0;
                    break;
                }
                case kMeshMaterialFaceIndices:
                {
                    uint32_t index = internal::stringToInt(token);
                    if (index <= emsize)
                        m_faces[nIndices++].index = index;
                    else
                        throw 6;
                    if (efisize <= nIndices)
                        state = kNone;
                    break;
                }
                case kMeshMaterialDeclation:
                {
                    currentMaterial = new XModelInternalMaterial;
                    currentMaterial->emmisive.setZero();
                    currentMaterial->index = 0;
                    currentMaterial->power = 0.0f;
                    currentMaterial->specular.setZero();
                    currentMaterial->textureName = 0;
                    materials.push_back(currentMaterial);
                    char *x = internal::stringToken(token, ";", &p);
                    char *y = internal::stringToken(NULL, ";", &p);
                    char *z = internal::stringToken(NULL, ";", &p);
                    char *w = internal::stringToken(NULL, ";", &p);
                    if (x && y && z && w) {
                        currentMaterial->color.setValue(internal::stringToFloat(x), internal::stringToFloat(y),
                                                        internal::stringToFloat(z), internal::stringToFloat(w));
                        currentMaterial->index = materialIndex++;
                        state = kMeshMaterialPower;
                    }
                    else {
                        throw 7;
                    }
                    break;
                }
                case kMeshMaterialPower:
                {
                    currentMaterial->power = internal::stringToFloat(token);
                    state = kMeshMaterialSpecularColor;
                    break;
                }
                case kMeshMaterialSpecularColor:
                {
                    char *x = internal::stringToken(token, ";", &p);
                    char *y = internal::stringToken(NULL, ";", &p);
                    char *z = internal::stringToken(NULL, ";", &p);
                    if (x && y && z) {
                        currentMaterial->specular.setValue(internal::stringToFloat(x), internal::stringToFloat(y),
                                                           internal::stringToFloat(z), 1.0f);
                        state = kMeshMaterialEmmisiveColor;
                    }
                    else {
                        throw 8;
                    }
                    break;
                }
                case kMeshMaterialEmmisiveColor:
                {
                    char *x = internal::stringToken(token, ";", &p);
                    char *y = internal::stringToken(NULL, ";", &p);
                    char *z = internal::stringToken(NULL, ";", &p);
                    if (x && y && z) {
                        currentMaterial->emmisive.setValue(internal::stringToFloat(x), internal::stringToFloat(y),
                                                           internal::stringToFloat(z), 1.0f);
                        state = kNone;
                    }
                    else {
                        throw 9;
                    }
                    break;
                }
                case kMeshTextureFilenameDeclation:
                {
                    size_t len = strlen(token);
                    char *filename = new char[len + 1];
                    strcpy(filename, token);
                    size_t i = 0, j = 0;
                    for (; i < len; i++) {
                        char c = token[i];
                        switch (c) {
                        case '"':
                        case ';':
                            break;
                        case '\\':
                            filename[j++] = '/';
                            break;
                        default:
                            filename[j++] = c;
                        }
                    }
                    filename[j] = 0;
                    currentMaterial->textureName = filename;
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
                        throw 10;
                    }
                    if (etsize <= static_cast<uint32_t>(m_coords.size()))
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
                        XModelInternalIndexedColor ic;
                        ic.index = internal::stringToInt(index);
                        ic.color.setValue(internal::stringToFloat(r), internal::stringToFloat(g),
                                          internal::stringToFloat(b), internal::stringToFloat(a));
                        colors.push_back(ic);
                    }
                    else {
                        throw 11;
                    }
                    if (ecsize <= static_cast<uint32_t>(colors.size())) {
                        const uint32_t nColors = colors.size();
                        m_colors.reserve(nColors);
                        nIndices = m_faces.size();
                        for (uint32_t i = 0; i < nColors; i++) {
                            XModelInternalIndexedColor &ic = colors[i];
                            if (ic.index < nIndices)
                                m_colors[ic.index] = ic.color;
                        }
                        state = kNone;
                    }
                    break;
                }
                default:
                {
                    throw 12;
                }
                }
            }
        }

        delete[] buffer;
        buffer = 0;

        if (materialIndex == emsize) {
            uint32_t size = m_faces.size();
            m_indices.reserve(emsize + 1);
            m_materials.reserve(emsize + 1);
            for (uint32_t i = 0; i <= emsize; i++)
                m_indices.push_back(new XModelIndexList);
            for (uint32_t i = 0; i <= emsize; i++)
                m_materials.push_back(new XMaterial);
            for (uint32_t i = 0; i < size; i++) {
                XModelFaceIndex &index = m_faces[i];
                XModelIndexList *indice = m_indices[index.index];
                btVector4 &v = index.value;
                if (index.count == 3) {
                    indice->push_back(static_cast<const uint16_t>(v.x()));
                    indice->push_back(static_cast<const uint16_t>(v.y()));
                    indice->push_back(static_cast<const uint16_t>(v.z()));
                }
                else if (index.count == 4) {
                    indice->push_back(static_cast<const uint16_t>(v.x()));
                    indice->push_back(static_cast<const uint16_t>(v.y()));
                    indice->push_back(static_cast<const uint16_t>(v.z()));
                    indice->push_back(static_cast<const uint16_t>(v.z()));
                    indice->push_back(static_cast<const uint16_t>(v.y()));
                    indice->push_back(static_cast<const uint16_t>(v.w()));
                }
            }
            size = materials.size();
            for (uint32_t i = 0; i < size; i++) {
                XModelInternalMaterial *internal = materials[i];
                uint32_t index = internal->index;
                XMaterial *material = new XMaterial(internal->color,
                                                    internal->specular,
                                                    internal->emmisive,
                                                    internal->textureName,
                                                    internal->power);
                delete m_materials[index];
                m_materials[index] = material;
            }
            ret = true;
        }

    } catch (int e) {
        fprintf(stderr, "%d", e);
    }

    internal::clearAll(materials);
    colors.clear();
    delete[] buffer;

    return ret;
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
    return &m_colors[0];
}

}
