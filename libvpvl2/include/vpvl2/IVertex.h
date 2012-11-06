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

#ifndef VPVL2_IVERTEX_H_
#define VPVL2_IVERTEX_H_

#include "vpvl2/Common.h"

namespace vpvl2
{
class IBone;

class VPVL2_API IVertex
{
public:
    enum Type {
        kBdef1,
        kBdef2,
        kBdef4,
        kSdef,
        kQdef,
        kMaxType
    };

    virtual ~IVertex() {}

    virtual void performSkinning(Vector3 &position, Vector3 &normal) const = 0;
    virtual void reset() = 0;

    virtual const Vector3 &origin() const = 0;
    virtual const Vector3 &normal() const = 0;
    virtual const Vector3 &textureCoord() const = 0;
    virtual const Vector4 &uv(int index) const = 0;
    virtual const Vector3 &delta() const = 0;
    virtual Type type() const = 0;
    virtual float edgeSize() const = 0;
    virtual float weight(int index) const = 0;
    virtual IBone *bone(int index) const = 0;
    virtual int index() const = 0;
    virtual void setOrigin(const Vector3 &value) = 0;
    virtual void setNormal(const Vector3 &value) = 0;
    virtual void setTextureCoord(const Vector3 &value) = 0;
    virtual void setUV(int index, const Vector4 &value) = 0;
    virtual void setType(Type value) = 0;
    virtual void setEdgeSize(float value) = 0;
    virtual void setWeight(int index, float weight) = 0;
    virtual void setBone(int index, IBone *value) = 0;
};

} /* namespace vpvl2 */

#endif
