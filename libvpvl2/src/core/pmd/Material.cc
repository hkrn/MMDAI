/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "vpvl2/vpvl2.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"
#include "vpvl2/pmd/Material.h"

#include "vpvl/Material.h"

namespace vpvl2
{
namespace pmd
{

const Color Material::kWhiteColor = Color(1, 1, 1, 1);

Material::Material(IModel *modelRef,
                   vpvl::Material *materialRef,
                   IEncoding *encodingRef,
                   const vpvl::PMDModel *originalModelRef,
                   int index)
    : m_modelRef(modelRef),
      m_materialRef(materialRef),
      m_encodingRef(encodingRef),
      m_mainTexture(0),
      m_sphereTexture(0),
      m_toonTexture(0),
      m_sphereTextureRenderMode(kNone),
      m_diffuse(materialRef->diffuse()),
      m_toonTextureIndex(-1),
      m_index(index)
{
    if (m_materialRef->isMainSphereAdd()) {
        m_sphereTexture = m_encodingRef->toString(m_materialRef->mainTextureName(),
                                                  IString::kShiftJIS, vpvl::Material::kNameSize);
        m_sphereTextureRenderMode = kAddTexture;
    }
    else if (m_materialRef->isMainSphereModulate()) {
        m_sphereTexture = m_encodingRef->toString(m_materialRef->mainTextureName(),
                                                  IString::kShiftJIS, vpvl::Material::kNameSize);
        m_sphereTextureRenderMode = kMultTexture;
    }
    else {
        m_mainTexture = m_encodingRef->toString(m_materialRef->mainTextureName(),
                                                IString::kShiftJIS, vpvl::Material::kNameSize);
    }
    if (m_materialRef->isSubSphereAdd()) {
        m_sphereTexture = m_encodingRef->toString(m_materialRef->subTextureName(),
                                                  IString::kShiftJIS, vpvl::Material::kNameSize);
        m_sphereTextureRenderMode = kAddTexture;
    }
    else if (m_materialRef->isSubSphereModulate()) {
        m_sphereTexture = m_encodingRef->toString(m_materialRef->subTextureName(),
                                                  IString::kShiftJIS, vpvl::Material::kNameSize);
        m_sphereTextureRenderMode = kMultTexture;
    }
    uint8_t toonIndex = m_materialRef->toonID();
    if (toonIndex > 0) {
        m_toonTexture = m_encodingRef->toString(originalModelRef->toonTexture(toonIndex - 1),
                                                IString::kShiftJIS, vpvl::PMDModel::kCustomTextureNameMax);
    }
    m_diffuse.setW(materialRef->opacity());
    m_indexRange.count = materialRef->countIndices();
}

Material::~Material()
{
    delete m_mainTexture;
    m_mainTexture = 0;
    delete m_sphereTexture;
    m_sphereTexture = 0;
    delete m_toonTexture;
    m_toonTexture = 0;
    m_modelRef = 0;
    m_materialRef = 0;
    m_encodingRef = 0;
    m_sphereTextureRenderMode = kNone;
    m_toonTextureIndex = -1;
    m_index = -1;
}

Color Material::ambient() const
{
    return m_materialRef->ambient();
}

Color Material::diffuse() const
{
    return m_diffuse;
}

Color Material::specular() const
{
    return m_materialRef->specular();
}

float Material::shininess() const
{
    return m_materialRef->shiness();
}

int Material::sizeofIndices() const
{
    return m_materialRef->countIndices();
}


bool Material::isSharedToonTextureUsed() const
{
    return false;
}

bool Material::isCullingDisabled() const
{
    return !btFuzzyZero(m_materialRef->opacity() - 1.0f);
}

bool Material::hasShadow() const
{
    return true;
}

bool Material::hasShadowMap() const
{
    return !btFuzzyZero(m_materialRef->opacity() - 0.98f);
}

bool Material::isSelfShadowEnabled() const
{
    return hasShadowMap();
}

bool Material::isEdgeEnabled() const
{
    return m_materialRef->isEdgeEnabled();
}

void Material::setAmbient(const Color &value)
{
    m_materialRef->setAmbient(value);
}

void Material::setDiffuse(const Color &value)
{
    m_materialRef->setDiffuse(value);
}

void Material::setSpecular(const Color &value)
{
    m_materialRef->setSpecular(value);
}

void Material::setIndexRange(const IndexRange &value)
{
    m_indexRange = value;
}

void Material::setShininess(float value)
{
    m_materialRef->setShiness(value);
}

void Material::setIndex(int value)
{
    m_index = value;
}

} /* namespace pmd */
} /* namespace vpvl2 */
