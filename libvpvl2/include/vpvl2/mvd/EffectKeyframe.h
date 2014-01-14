/**

 Copyright (c) 2010-2014  hkrn

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
#ifndef VPVL2_MVD_EFFECTKEYFRAME_H_
#define VPVL2_MVD_EFFECTKEYFRAME_H_

#include "vpvl2/IEffectKeyframe.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/internal/Keyframe.h"

namespace vpvl2
{
class IEncoding;

namespace mvd
{

class VPVL2_API EffectKeyframe VPVL2_DECL_FINAL : public IEffectKeyframe
{
public:
    EffectKeyframe(const Motion *motionRef);
    ~EffectKeyframe();

    static vsize size();
    static bool preparse(uint8 *&ptr, vsize &rest, vsize reserved, Motion::DataInfo &info);

    void read(const uint8 *data);
    void write(uint8 *data) const;
    vsize estimateSize() const;
    IEffectKeyframe *clone() const;
    void setName(const IString *value);
    Type type() const;
    const Motion *parentMotionRef() const;

    VPVL2_KEYFRAME_DEFINE_METHODS()
    bool isVisible() const;
    bool isAddBlendEnabled() const;
    bool isShadowEnabled() const;
    float32 scaleFactor() const;
    float32 opacity() const;
    IModel *parentModelRef() const;
    IBone *parentBoneRef() const;
    void setVisible(bool value);
    void setAddBlendEnable(bool value);
    void setShadowEnable(bool value);
    void setScaleFactor(float32 value);
    void setOpacity(float32 value);
    void setParentModelRef(IModel *value);
    void setParentBoneRef(IBone *value);

private:
    VPVL2_KEYFRAME_DEFINE_FIELDS()
    mutable EffectKeyframe *m_ptr;
    const Motion *m_motionRef;
    IModel *m_parentModelRef;
    IBone *m_parentBoneRef;
    float32 m_scaleFactor;
    float32 m_opacity;
    bool m_visible;
    bool m_addBlend;
    bool m_shadow;

    VPVL2_DISABLE_COPY_AND_ASSIGN(EffectKeyframe)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif
