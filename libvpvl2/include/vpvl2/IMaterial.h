/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef VPVL2_IMATERIAL_H_
#define VPVL2_IMATERIAL_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IString;

class VPVL2_API IMaterial
{
public:
    enum SphereTextureRenderMode {
        kNone,
        kMultTexture,
        kAddTexture,
        kSubTexture,
        kMaxSphereTextureRenderModeType
    };

    virtual ~IMaterial() {}

    /**
     * 親のモデルのインスタンスを返します.
     *
     * @brief parentModelRef
     * @return IModel
     */
    virtual IModel *parentModelRef() const = 0;

    virtual const IString *name() const = 0;
    virtual const IString *englishName() const = 0;
    virtual const IString *userDataArea() const = 0;
    virtual const IString *mainTexture() const = 0;
    virtual const IString *sphereTexture() const = 0;
    virtual const IString *toonTexture() const = 0;
    virtual SphereTextureRenderMode sphereTextureRenderMode() const = 0;
    virtual const Color &ambient() const = 0;
    virtual const Color &diffuse() const = 0;
    virtual const Color &specular() const = 0;
    virtual const Color &edgeColor() const = 0;
    virtual const Color &mainTextureBlend() const = 0;
    virtual const Color &sphereTextureBlend() const = 0;
    virtual const Color &toonTextureBlend() const = 0;
    virtual float shininess() const = 0;
    virtual float edgeSize() const = 0;
    virtual int index() const = 0;
    virtual int textureIndex() const = 0;
    virtual int sphereTextureIndex() const = 0;
    virtual int toonTextureIndex() const = 0;
    virtual int sizeofIndices() const = 0;
    virtual bool isSharedToonTextureUsed() const = 0;
    virtual bool isCullFaceDisabled() const = 0;
    virtual bool hasShadow() const = 0;
    virtual bool isShadowMapDrawn() const = 0;
    virtual bool isSelfShadowDrawn() const = 0;
    virtual bool isEdgeDrawn() const = 0;

    virtual void setName(const IString *value) = 0;
    virtual void setEnglishName(const IString *value) = 0;
    virtual void setUserDataArea(const IString *value) = 0;
    virtual void setMainTexture(const IString *value) = 0;
    virtual void setSphereTexture(const IString *value) = 0;
    virtual void setToonTexture(const IString *value) = 0;
    virtual void setSphereTextureRenderMode(SphereTextureRenderMode value) = 0;
    virtual void setAmbient(const Color &value) = 0;
    virtual void setDiffuse(const Color &value) = 0;
    virtual void setSpecular(const Color &value) = 0;
    virtual void setEdgeColor(const Color &value) = 0;
    virtual void setShininess(float value) = 0;
    virtual void setEdgeSize(float value) = 0;
    virtual void setMainTextureIndex(int value) = 0;
    virtual void setSphereTextureIndex(int value) = 0;
    virtual void setToonTextureIndex(int value) = 0;
    virtual void setIndices(int value) = 0;
    virtual void setFlags(int value) = 0;
};

} /* namespace vpvl2 */

#endif
