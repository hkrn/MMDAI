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

#include "vpvl/Common.h"

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

class VPVL_API Material
{
public:
    Material();
    ~Material();

    static const int kNameSize = 20;

    static size_t stride();

    void read(const uint8_t *data);
    void write(uint8_t *data) const;

    const uint8_t *rawName() const {
        return m_rawName;
    }
    const uint8_t *mainTextureName() const {
        return m_mainTextureName;
    }
    const uint8_t *subTextureName() const {
        return m_subTextureName;
    }
    const Color &ambient() const {
        return m_ambient;
    }
    const Color &diffuse() const {
        return m_diffuse;
    }
    const Color &specular() const {
        return m_specular;
    }
    float opacity() const {
        return m_opacity;
    }
    float shiness() const {
        return m_shiness;
    }
    int countIndices() const {
        return m_nindices;
    }
    uint8_t toonID() const {
        return m_toonID;
    }
    bool isEdgeEnabled() const {
        return m_edge;
    }
    bool isMainSphereModulate() const {
        return m_mainSphereModulate;
    }
    bool isMainSphereAdd() const {
        return m_mainSphereAdd;
    }
    bool isSubSphereModulate() const {
        return m_subSphereModulate;
    }
    bool isSubSphereAdd() const {
        return m_subSphereAdd;
    }

    void setMainTextureName(const uint8_t *value) {
        copyBytesSafe(m_mainTextureName, value, sizeof(m_mainTextureName));
    }
    void setSubTextureName(const uint8_t *value) {
        copyBytesSafe(m_subTextureName, value, sizeof(m_subTextureName));
    }
    void setAmbient(const Color &value) {
        m_ambient = value;
    }
    void setDiffuse(const Color &value) {
        m_diffuse = value;
    }
    void setSpecular(const Color &value) {
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
    uint8_t m_rawName[kNameSize + 1];
    uint8_t m_mainTextureName[kNameSize + 1];
    uint8_t m_subTextureName[kNameSize + 1];
    Color m_ambient;
    Color m_diffuse;
    Color m_specular;
    float m_opacity;
    float m_shiness;
    int m_nindices;
    uint8_t m_toonID;
    bool m_edge;
    bool m_mainSphereModulate;
    bool m_mainSphereAdd;
    bool m_subSphereModulate;
    bool m_subSphereAdd;

    VPVL_DISABLE_COPY_AND_ASSIGN(Material)
};

typedef Array<Material*> MaterialList;

} /* namespace vpvl */

#endif
