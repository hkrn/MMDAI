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
#ifndef VPVL2_VMD_MODELKEYFRAME_H_
#define VPVL2_VMD_MODELKEYFRAME_H_

#include "vpvl2/IModelKeyframe.h"
#include "vpvl2/internal/Keyframe.h"

namespace vpvl2
{
class IEncoding;

namespace vmd
{

class VPVL2_API ModelKeyframe VPVL2_DECL_FINAL : public IModelKeyframe
{
public:
    static const int kNameSize = 20;
    static bool preparse(uint8 *&ptr, vsize &rest, const int32 nkeyframes);

    ModelKeyframe(IEncoding *encoding);
    ~ModelKeyframe();

    void read(const uint8 *data);
    void write(uint8 *data) const;
    void updateInverseKinematics(IModel *model) const;
    vsize estimateSize() const;
    IModelKeyframe *clone() const;

    VPVL2_KEYFRAME_DEFINE_METHODS()
    Type type() const;
    void setName(const IString *name);

    bool isVisible() const;
    bool isShadowEnabled() const;
    bool isAddBlendEnabled() const;
    bool isPhysicsEnabled() const;
    bool isInverseKinematicsEnabld(const IBone *value) const;
    uint8 physicsStillMode() const;
    IVertex::EdgeSizePrecision edgeWidth() const;
    Color edgeColor() const;
    void setVisible(bool value);
    void setShadowEnable(bool value);
    void setAddBlendEnable(bool value);
    void setPhysicsEnable(bool value);
    void setPhysicsStillMode(uint8 value);
    void setEdgeWidth(const IVertex::EdgeSizePrecision &value);
    void setEdgeColor(const Color &value);
    void setInverseKinematicsEnable(IBone *bone, bool value);

private:
    VPVL2_KEYFRAME_DEFINE_FIELDS()
    struct IKState {
        IKState(IString *n, bool e) : name(n), enabled(e) {}
        ~IKState() { delete name; name = 0; enabled = false; }
        IString *name;
        bool enabled;
    };
    PointerHash<HashPtr, IKState> m_states;
    IEncoding *m_encodingRef;
    bool m_visible;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ModelKeyframe)
};

} /* namespace vmd */
} /* namespace vpvl2 */

#endif
