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

#include <ctype.h>

namespace vpvl
{

struct XModelToken {
    const uint8_t *ptr;
    size_t size;
};

enum XModelParseState
{
    kNone,
    kHeaderDeclation,
    kMeshDeclation,
    kMeshVertices,
    kMeshFacesSize,
    kMeshFaces,
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
    kMeshTextureCoords
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
    strtok_r(m_buffer, " \r\n\t", &p); // signature
    strtok_r(NULL, " \r\n\t", &p); // version
    strtok_r(NULL, " \r\n\t", &p); // size
    while ((token = strtok_r(NULL, " \r\n\t", &p))) {
        if (skip && *token == '}') {
            skip = false;
            continue;
        }
        else if (!strcmp(token, "template")) {
            skip = true;
        }
        if (skip)
            continue;
        tokens.push_back(token);
    }

    btVector4 color, specular, emmisive;
    float power = 0;

    int evsize = 0, efsize = 0, emsize = 0, efisize = 0, etsize = 0;
    int ntokens = tokens.size(), depth = 0, mindex = -1;
    XModelParseState state = kNone;
    for (int i = 0; i < ntokens; i++) {
        char *token = tokens[i];
        if (*token == '{') {
            switch (state) {
            case kHeaderDeclation:
            case kMeshDeclation:
            case kMeshMaterialListDeclation:
            case kMeshMaterialDeclation:
            case kMeshTextureFilenameDeclation:
            case kMeshTextureCoordsDeclation:
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
                if (depth == 0 && !strncmp(token, "Header", 6))
                    state = kHeaderDeclation;
                else if (depth == 0 && !strncmp(token, "Mesh", 4))
                    state = kMeshDeclation;
                else if (depth == 1 && !strncmp(token, "MeshMaterialList", 16))
                    state = kMeshMaterialListDeclation;
                else if (depth == 2 && !strncmp(token, "Material", 8))
                    state = kMeshMaterialDeclation;
                else if (depth == 3 && !strncmp(token, "TextureFilename", 15))
                    state = kMeshTextureFilenameDeclation;
                else if (depth == 1 && !strncmp(token, "MeshTextureCoords", 17))
                    state = kMeshTextureCoordsDeclation;
                else
                    return false;
                depth++;
                break;
            }
            case kHeaderDeclation:
            {
                continue;
            }
            case kMeshDeclation:
            {
                evsize = atoi(token);
                state = kMeshVertices;
                break;
            }
            case kMeshVertices:
            {
                char *x = strtok_r(token, ";", &p);
                char *y = strtok_r(NULL, ";", &p);
                char *z = strtok_r(NULL, ";", &p);
                if (x && y && z) {
                    btVector3 v(strtof(x, &p), strtof(y, &p), strtof(z, &p));
                    m_vertices.push_back(v);
                }
                else {
                    return false;
                }
                if (evsize <= m_vertices.size())
                    state = kMeshFacesSize;
                break;
            }
            case kMeshFacesSize:
            {
                efsize = atoi(token);
                state = kMeshFaces;
                break;
            }
            case kMeshFaces:
            {
                char *s = strtok_r(token, ";", &p);
                char *x = strtok_r(NULL, ",", &p);
                char *y = strtok_r(NULL, ",", &p);
                char *z = strtok_r(NULL, ",", &p);
                char *w = strtok_r(NULL, ",", &p);
                if (s) {
                    int size = atoi(s);
                    if (size == 3 && x && y && z) {
                        btVector4 v(atoi(x), atoi(y), atoi(z), 0.0f);
                        m_faces.push_back(v);
                    }
                    else if (size == 4 && x && y && z && w) {
                        btVector4 v(atoi(x), atoi(y), atoi(z), atoi(w));
                        m_faces.push_back(v);
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return false;
                }
                if (efsize <= m_faces.size())
                    state = kNone;
                break;
            }
            case kMeshMaterialListDeclation:
            {
                emsize = atoi(token);
                state = kMeshMaterialFaceIndicesSize;
                break;
            }
            case kMeshMaterialFaceIndicesSize:
            {
                efisize = atoi(token);
                state = kMeshMaterialFaceIndices;
                break;
            }
            case kMeshMaterialFaceIndices:
            {
                int index = atoi(token);
                if (index < efsize)
                    m_indices.push_back(index);
                if (efisize <= m_indices.size())
                    state = kNone;
                break;
            }
            case kMeshMaterialDeclation:
            {
                char *x = strtok_r(token, ";", &p);
                char *y = strtok_r(NULL, ";", &p);
                char *z = strtok_r(NULL, ";", &p);
                char *w = strtok_r(NULL, ";", &p);
                if (x && y && z && w) {
                    color.setValue(strtof(x, &p), strtof(y, &p), strtof(z, &p), strtof(w, &p));
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
                power = strtof(token, &p);
                state = kMeshMaterialSpecularColor;
                break;
            }
            case kMeshMaterialSpecularColor:
            {
                char *x = strtok_r(token, ";", &p);
                char *y = strtok_r(NULL, ";", &p);
                char *z = strtok_r(NULL, ";", &p);
                if (x && y && z) {
                    specular.setValue(strtof(x, &p), strtof(y, &p), strtof(z, &p), 1.0f);
                    state = kMeshMaterialEmmisiveColor;
                }
                else {
                    return false;
                }
                break;
            }
            case kMeshMaterialEmmisiveColor:
            {
                char *x = strtok_r(token, ";", &p);
                char *y = strtok_r(NULL, ";", &p);
                char *z = strtok_r(NULL, ";", &p);
                if (x && y && z) {
                    emmisive.setValue(strtof(x, &p), strtof(y, &p), strtof(z, &p), 1.0f);
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
                etsize = atoi(token);
                state = kMeshTextureCoords;
                break;
            }
            case kMeshTextureCoords:
            {
                char *x = strtok_r(token, ";", &p);
                char *y = strtok_r(NULL, ";", &p);
                if (x && y) {
                    btVector3 v(strtof(x, &p), strtof(y, &p), 0.0f);
                    m_coords.push_back(v);
                }
                else {
                    return false;
                }
                if (etsize <= m_coords.size())
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

}
