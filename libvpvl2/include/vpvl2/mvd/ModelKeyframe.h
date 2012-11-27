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

#ifndef VPVL2_MVD_MODELKEYFRAME_H_
#define VPVL2_MVD_MODELKEYFRAME_H_

#include "vpvl2/IModelKeyframe.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/vmd/BaseKeyframe.h"

namespace vpvl2
{
class IBone;
class IEncoding;

namespace mvd
{

class VPVL2_API ModelKeyframe : public vmd::BaseKeyframe, public IModelKeyframe
{
public:
    ModelKeyframe(const Motion *motionRef, int countOfIKBones);
    ~ModelKeyframe();

    static size_t size();
    static bool preparse(uint8_t *&ptr, size_t &rest, size_t reserved, size_t countOfIK, Motion::DataInfo &info);

    void read(const uint8_t *data);
    void write(uint8_t *data) const;
    size_t estimateSize() const;
    IModelKeyframe *clone() const;
    const Motion *parentMotionRef() const;
    void mergeIKState(const Hash<btHashInt, vpvl2::IBone *> &bones) const;
    void setIKState(const Hash<btHashInt, vpvl2::IBone *> &bones);

    bool isVisible() const;
    bool isShadowEnabled() const;
    bool isAddBlendEnabled() const;
    bool isPhysicsEnabled() const;
    uint8_t physicsStillMode() const;
    Scalar edgeWidth() const;
    Color edgeColor() const;
    void setVisible(bool value);
    void setShadowEnable(bool value);
    void setAddBlendEnable(bool value);
    void setPhysicsEnable(bool value);
    void setPhysicsStillMode(uint8_t value);
    void setEdgeWidth(const Scalar &value);
    void setEdgeColor(const Color &value);

    void setName(const IString *value);
    Type type() const;

private:
    mutable ModelKeyframe *m_ptr;
    const Motion *m_motionRef;
    Array<bool> m_bonesOfIK;
    Color m_edgeColor;
    Scalar m_edgeWidth;
    int m_countOfIKBones;
    uint8_t m_physicsStillMode;
    bool m_visible;
    bool m_shadow;
    bool m_addBlend;
    bool m_physics;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ModelKeyframe)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif

