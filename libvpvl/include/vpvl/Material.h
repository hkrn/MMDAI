/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_MATERIAL_H_
#define VPVL_MATERIAL_H_

#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

typedef struct MaterialPrivate MaterialPrivate;

class Material
{
public:
    Material();
    ~Material();

    static size_t stride(const char *data);

    void read(const char *data);

    const char *rawName() const {
        return m_rawName;
    }
    const char *primaryTextureName() const {
        return m_primaryTextureName;
    }
    const char *secondTextureName() const {
        return m_secondTextureName;
    }
    const btVector4 &ambient() const {
        return m_ambient;
    }
    const btVector4 &averageColor() const {
        return m_averageColor;
    }
    const btVector4 &diffuse() const {
        return m_diffuse;
    }
    const btVector4 &specular() const {
        return m_specular;
    }
    float alpha() const {
        return m_alpha;
    }
    float shiness() const {
        return m_shiness;
    }
    uint32_t countIndices() const {
        return m_nindices;
    }
    uint8_t toonID() const {
        return m_toonID;
    }
    bool isEdgeEnabled() const {
        return m_edge;
    }
    bool isSpherePrimary() const {
        return m_firstSPH;
    }
    bool isSphereAuxPrimary() const {
        return m_firstSPA;
    }
    bool isSphereSecond() const {
        return m_secondSPH;
    }
    bool isSphereAuxSecond() const {
        return m_secondSPA;
    }
    MaterialPrivate *privateData() const {
        return m_private;
    }

    void setPrimaryTextureName(const char *value) {
        stringCopySafe(m_primaryTextureName, value, sizeof(m_primaryTextureName));
    }
    void setSecondTextureName(const char *value) {
        stringCopySafe(m_secondTextureName, value, sizeof(m_secondTextureName));
    }
    void setAmbient(const btVector4 &value) {
        m_ambient = value;
    }
    void setAverageColor(const btVector4 &value) {
        m_averageColor = value;
    }
    void setDiffuse(const btVector4 &value) {
        m_diffuse = value;
    }
    void setSpecular(const btVector4 &value) {
        m_specular = value;
    }
    void setAlpha(float value) {
        m_alpha = value;
    }
    void setShiness(float value) {
        m_shiness = value;
    }
    void setEdgeEnabled(bool value) {
        m_edge = value;
    }
    void setPrivateData(MaterialPrivate *value) {
        m_private = value;
    }

private:
    char m_rawName[20];
    char m_primaryTextureName[20];
    char m_secondTextureName[20];
    btVector4 m_ambient;
    btVector4 m_averageColor;
    btVector4 m_diffuse;
    btVector4 m_specular;
    float m_alpha;
    float m_shiness;
    uint32_t m_nindices;
    uint8_t m_toonID;
    bool m_edge;
    bool m_firstSPH;
    bool m_firstSPA;
    bool m_secondSPH;
    bool m_secondSPA;
    MaterialPrivate *m_private;
};

typedef btAlignedObjectArray<Material*> MaterialList;

} /* namespace vpvl */

#endif
