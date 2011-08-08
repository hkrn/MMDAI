/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
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

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Material class represents a material of a Polygon Model Data object.
 */

class VPVL_EXPORT Material
{
public:
    Material();
    ~Material();

    static const int kNameSize = 20;

    static size_t stride();

    void read(const uint8_t *data);

    const uint8_t *rawName() const {
        return m_rawName;
    }
    const uint8_t *mainTextureName() const {
        return m_mainTextureName;
    }
    const uint8_t *subTextureName() const {
        return m_subTextureName;
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
    float opacity() const {
        return m_opacity;
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
    bool isMultiplicationSphereMain() const {
        return m_isMultiplicationSphereMain;
    }
    bool isAdditionalSphereMain() const {
        return m_isAdditionalSphereMain;
    }
    bool isMultiplicationSphereSecond() const {
        return m_isMultiplicationSphereSub;
    }
    bool isAdditionalSphereSub() const {
        return m_isAdditionalSphereSub;
    }

    void setMainTextureName(const uint8_t *value) {
        copyBytesSafe(m_mainTextureName, value, sizeof(m_mainTextureName));
    }
    void setSubTextureName(const uint8_t *value) {
        copyBytesSafe(m_subTextureName, value, sizeof(m_subTextureName));
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
    void setOpacity(float value) {
        m_opacity = value;
    }
    void setShiness(float value) {
        m_shiness = value;
    }
    void setEdgeEnabled(bool value) {
        m_edge = value;
    }

private:
    uint8_t m_rawName[kNameSize];
    uint8_t m_mainTextureName[kNameSize];
    uint8_t m_subTextureName[kNameSize];
    btVector4 m_ambient;
    btVector4 m_averageColor;
    btVector4 m_diffuse;
    btVector4 m_specular;
    float m_opacity;
    float m_shiness;
    uint32_t m_nindices;
    uint8_t m_toonID;
    bool m_edge;
    bool m_isMultiplicationSphereMain;
    bool m_isAdditionalSphereMain;
    bool m_isMultiplicationSphereSub;
    bool m_isAdditionalSphereSub;

    VPVL_DISABLE_COPY_AND_ASSIGN(Material)
};

typedef btAlignedObjectArray<Material*> MaterialList;

} /* namespace vpvl */

#endif
