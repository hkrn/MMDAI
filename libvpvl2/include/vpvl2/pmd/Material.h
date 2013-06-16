/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_PMD_MATERIAL_H_
#define VPVL2_PMD_MATERIAL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IMaterial.h"
#include "vpvl2/pmd/Model.h"

namespace vpvl
{
class Material;
class PMDModel;
}

namespace vpvl2
{

namespace pmd
{

class VPVL2_API Material : public IMaterial
{
public:
    static const int kNameSize = 20;

    Material(IModel *modelRef,
             vpvl::Material *materialRef,
             IEncoding *encodingRef,
             const vpvl::PMDModel *originalModelRef,
             int index);
    ~Material();

    IModel *parentModelRef() const { return m_modelRef; }
    const IString *name() const { return 0; }
    const IString *englishName() const { return 0; }
    const IString *userDataArea() const { return 0; }
    const IString *mainTexture() const { return m_mainTexture; }
    const IString *sphereTexture() const { return m_sphereTexture; }
    const IString *toonTexture() const { return m_toonTexture; }
    SphereTextureRenderMode sphereTextureRenderMode() const { return m_sphereTextureRenderMode; }
    Color ambient() const;
    Color diffuse() const;
    Color specular() const;
    Color edgeColor() const { return kZeroC; }
    Color mainTextureBlend() const { return kWhiteColor; }
    Color sphereTextureBlend() const { return kWhiteColor; }
    Color toonTextureBlend() const { return kWhiteColor; }
    IndexRange indexRange() const { return m_indexRange; }
    float shininess() const;
    IVertex::EdgeSizePrecision edgeSize() const { return 1; }
    int index() const { return m_index; }
    int textureIndex() const { return -1; }
    int sphereTextureIndex() const { return -1; }
    int toonTextureIndex() const { return m_toonTextureIndex; }
    int sizeofIndices() const;
    bool isSharedToonTextureUsed() const;
    bool isCullingDisabled() const;
    bool hasShadow() const;
    bool hasShadowMap() const;
    bool isSelfShadowEnabled() const;
    bool isEdgeEnabled() const;

    void setName(const IString * /* value */) {}
    void setEnglishName(const IString * /* value */) {}
    void setUserDataArea(const IString * /* value */) {}
    void setMainTexture(const IString * /* value */) {}
    void setSphereTexture(const IString * /* value */) {}
    void setToonTexture(const IString * /* value */) {}
    void setSphereTextureRenderMode(SphereTextureRenderMode /*value*/) {}
    void setAmbient(const Color &value);
    void setDiffuse(const Color &value);
    void setSpecular(const Color &value);
    void setEdgeColor(const Color & /* value */) {}
    void setIndexRange(const IndexRange &value);
    void setShininess(float value);
    void setEdgeSize(const IVertex::EdgeSizePrecision & /* value */) {}
    void setMainTextureIndex(int /* value */) {}
    void setSphereTextureIndex(int /* value */) {}
    void setToonTextureIndex(int /* value */) {}
    void setIndices(int /* value */) {}
    void setFlags(int /* value */) {}
    void setIndex(int value);

private:
    static const Color kWhiteColor;
    IModel *m_modelRef;
    vpvl::Material *m_materialRef;
    IEncoding *m_encodingRef;
    IString *m_mainTexture;
    IString *m_sphereTexture;
    IString *m_toonTexture;
    SphereTextureRenderMode m_sphereTextureRenderMode;
    Color m_diffuse;
    IndexRange m_indexRange;
    int m_toonTextureIndex;
    int m_index;
};

} /* namespace pmd */
} /* namespace vpvl2 */

#endif
