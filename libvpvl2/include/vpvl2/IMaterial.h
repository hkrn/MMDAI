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
#ifndef VPVL2_IMATERIAL_H_
#define VPVL2_IMATERIAL_H_

#include "vpvl2/IEncoding.h"
#include "vpvl2/IVertex.h"

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
    enum Flags {
        kDisableCulling   = 0x1,
        kHasShadow        = 0x2,
        kHasShadowMap     = 0x4,
        kEnableSelfShadow = 0x8,
        kEnableEdge       = 0x10,
        kHasVertexColor   = 0x20,
        kEnablePointDraw  = 0x40,
        kEnableLineDraw   = 0x80,
        kMaxMaterialFlags = 0x100
    };

    struct IndexRange {
        IndexRange()
            : start(0),
              end(0),
              count(0)
        {
        }
        bool operator ==(const IndexRange &value) const {
            return start == value.start && end == value.end && count == value.count;
        }
        bool operator !=(const IndexRange &value) const {
            return !(*this == value);
        }
        int start;
        int end;
        int count;
    };
    class PropertyEventListener {
    public:
        virtual ~PropertyEventListener() {}
        virtual void nameWillChange(const IString *value, IEncoding::LanguageType type, IMaterial *material) = 0;
        virtual void userDataAreaWillChange(const IString *value, IMaterial *material) = 0;
        virtual void mainTextureWillChange(const IString *value, IMaterial *material) = 0;
        virtual void sphereTextureWillChange(const IString *value, IMaterial *material) = 0;
        virtual void toonTextureWillChange(const IString *value, IMaterial *material) = 0;
        virtual void sphereTextureRenderModeWillChange(SphereTextureRenderMode value, IMaterial *material) = 0;
        virtual void ambientWillChange(const Color &value, IMaterial *material) = 0;
        virtual void diffuseWillChange(const Color &value, IMaterial *material) = 0;
        virtual void specularWillChange(const Color &value, IMaterial *material) = 0;
        virtual void edgeColorWillChange(const Color &value, IMaterial *material) = 0;
        virtual void indexRangeWillChange(const IndexRange &value, IMaterial *material) = 0;
        virtual void shininessWillChange(float32 value, IMaterial *material) = 0;
        virtual void edgeSizeWillChange(const IVertex::EdgeSizePrecision &value, IMaterial *material) = 0;
        virtual void mainTextureIndexWillChange(int value, IMaterial *material) = 0;
        virtual void sphereTextureIndexWillChange(int value, IMaterial *material) = 0;
        virtual void toonTextureIndexWillChange(int value, IMaterial *material) = 0;
        virtual void flagsWillChange(int value, IMaterial *material) = 0;
    };

    virtual ~IMaterial() {}

    virtual void addEventListenerRef(PropertyEventListener *value) = 0;
    virtual void removeEventListenerRef(PropertyEventListener *value) = 0;
    virtual void getEventListenerRefs(Array<PropertyEventListener *> &value) = 0;

    /**
     * 親のモデルのインスタンスを返します.
     *
     * @brief parentModelRef
     * @return IModel
     */
    virtual IModel *parentModelRef() const = 0;

    virtual const IString *name(IEncoding::LanguageType type) const = 0;
    virtual const IString *userDataArea() const = 0;
    virtual const IString *mainTexture() const = 0;
    virtual const IString *sphereTexture() const = 0;
    virtual const IString *toonTexture() const = 0;
    virtual SphereTextureRenderMode sphereTextureRenderMode() const = 0;
    virtual Color ambient() const = 0;
    virtual Color diffuse() const = 0;
    virtual Color specular() const = 0;
    virtual Color edgeColor() const = 0;
    virtual Color mainTextureBlend() const = 0;
    virtual Color sphereTextureBlend() const = 0;
    virtual Color toonTextureBlend() const = 0;
    virtual IndexRange indexRange() const = 0;
    virtual float32 shininess() const = 0;
    virtual IVertex::EdgeSizePrecision edgeSize() const = 0;
    virtual int index() const = 0;
    virtual int textureIndex() const = 0;
    virtual int sphereTextureIndex() const = 0;
    virtual int toonTextureIndex() const = 0;
    virtual bool isSharedToonTextureUsed() const = 0;
    virtual bool isCullingDisabled() const = 0;
    virtual bool hasShadow() const = 0;
    virtual bool hasShadowMap() const = 0;
    virtual bool isSelfShadowEnabled() const = 0;
    virtual bool isEdgeEnabled() const = 0;

    virtual void setName(const IString *value, IEncoding::LanguageType type) = 0;
    virtual void setUserDataArea(const IString *value) = 0;
    virtual void setMainTexture(const IString *value) = 0;
    virtual void setSphereTexture(const IString *value) = 0;
    virtual void setToonTexture(const IString *value) = 0;
    virtual void setSphereTextureRenderMode(SphereTextureRenderMode value) = 0;
    virtual void setAmbient(const Color &value) = 0;
    virtual void setDiffuse(const Color &value) = 0;
    virtual void setSpecular(const Color &value) = 0;
    virtual void setEdgeColor(const Color &value) = 0;
    virtual void setIndexRange(const IndexRange &value) = 0;
    virtual void setShininess(float32 value) = 0;
    virtual void setEdgeSize(const IVertex::EdgeSizePrecision &value) = 0;
    virtual void setMainTextureIndex(int value) = 0;
    virtual void setSphereTextureIndex(int value) = 0;
    virtual void setToonTextureIndex(int value) = 0;
    virtual void setFlags(int value) = 0;
};

} /* namespace vpvl2 */

#endif
