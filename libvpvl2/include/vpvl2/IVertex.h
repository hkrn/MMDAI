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
#ifndef VPVL2_IVERTEX_H_
#define VPVL2_IVERTEX_H_

#include "vpvl2/Common.h"

namespace vpvl2
{
class IBone;
class IMaterial;
class IModel;

class VPVL2_API IVertex
{
public:
#ifdef VPVL2_ENABLE_GLES2
    typedef float32 EdgeSizePrecision;
    typedef float32 WeightPrecision;
#else
    typedef float64 EdgeSizePrecision;
    typedef float64 WeightPrecision;
#endif
    enum Type {
        kBdef1,
        kBdef2,
        kBdef4,
        kSdef,
        kQdef,
        kMaxType
    };

    class PropertyEventListener {
    public:
        virtual ~PropertyEventListener() {}
        virtual void originWillChange(const Vector3 &value, IVertex *vertex) = 0;
        virtual void normalWillChange(const Vector3 &value, IVertex *vertex) = 0;
        virtual void textureCoordWillChange(const Vector3 &value, IVertex *vertex) = 0;
        virtual void UVWillChange(int index, const Vector4 &value, IVertex *vertex) = 0;
        virtual void typeWillChange(Type value, IVertex *vertex) = 0;
        virtual void edgeSizeWillChange(const EdgeSizePrecision &value, IVertex *vertex) = 0;
        virtual void weightWillChange(int index, const WeightPrecision &weight, IVertex *vertex) = 0;
        virtual void boneRefWillChange(int index, IBone *value, IVertex *vertex) = 0;
        virtual void materialRefWillChange(IMaterial *value, IVertex *vertex) = 0;
    };

    virtual ~IVertex() {}

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

    virtual void performSkinning(Vector3 &position, Vector3 &normal) const = 0;
    virtual void reset() = 0;

    virtual Vector3 origin() const = 0;
    virtual Vector3 normal() const = 0;
    virtual Vector3 textureCoord() const = 0;
    virtual Vector4 uv(int index) const = 0;
    virtual Vector3 delta() const = 0;
    virtual Type type() const = 0;
    virtual EdgeSizePrecision edgeSize() const = 0;
    virtual WeightPrecision weight(int index) const = 0;
    virtual IBone *boneRef(int index) const = 0;
    virtual IMaterial *materialRef() const = 0;
    virtual int index() const = 0;
    virtual void setOrigin(const Vector3 &value) = 0;
    virtual void setNormal(const Vector3 &value) = 0;
    virtual void setTextureCoord(const Vector3 &value) = 0;
    virtual void setUV(int index, const Vector4 &value) = 0;
    virtual void setType(Type value) = 0;
    virtual void setEdgeSize(const EdgeSizePrecision &value) = 0;
    virtual void setWeight(int index, const WeightPrecision &weight) = 0;
    virtual void setBoneRef(int index, IBone *value) = 0;
    virtual void setMaterialRef(IMaterial *value) = 0;
};

} /* namespace vpvl2 */

#endif
