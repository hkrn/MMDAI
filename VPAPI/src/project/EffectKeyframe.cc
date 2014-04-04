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

#include <vpvl2/vpvl2.h>
#include <project/EffectKeyframe.h>
#include <project/Motion.h>

using namespace vpvl2;

namespace project
{

struct EffectKeyframe::PrivateContext {
    PrivateContext(Motion *parent, int id)
        : parentMotionRef(parent),
          rowID(id)
    {
    }
    ~PrivateContext() {
        parentMotionRef = 0;
        rowID = -1;
    }

    Motion *parentMotionRef;
    int rowID;
};

EffectKeyframe::EffectKeyframe(Motion *parent, int rowID)
    : m_context(new PrivateContext(parent, rowID))
{
}

EffectKeyframe::~EffectKeyframe()
{
    delete m_context;
    m_context = 0;
}

void EffectKeyframe::read(const uint8 *data)
{
}

void EffectKeyframe::write(uint8 *data) const
{
}

vsize EffectKeyframe::estimateSize() const
{
    return 0;
}

const IString *EffectKeyframe::name() const
{
    return 0;
}

IKeyframe::TimeIndex EffectKeyframe::timeIndex() const
{
    return 0;
}

IKeyframe::LayerIndex EffectKeyframe::layerIndex() const
{
    return 0;
}
void EffectKeyframe::setName(const IString *value)
{
}

void EffectKeyframe::setTimeIndex(const TimeIndex &value)
{
}

void EffectKeyframe::setLayerIndex(const LayerIndex &value)
{
}

IKeyframe::Type EffectKeyframe::type() const
{
    return kEffectKeyframe;
}

IEffectKeyframe *EffectKeyframe::clone() const
{
    return 0;
}

bool EffectKeyframe::isVisible() const
{
    return false;
}

bool EffectKeyframe::isAddBlendEnabled() const
{
    return false;
}

bool EffectKeyframe::isShadowEnabled() const
{
    return false;
}

float32 EffectKeyframe::scaleFactor() const
{
    return 0;
}

float32 EffectKeyframe::opacity() const
{
    return 0;
}

IModel *EffectKeyframe::parentModelRef() const
{
    return 0;
}

IBone *EffectKeyframe::parentBoneRef() const
{
    return 0;
}

void EffectKeyframe::setVisible(bool value)
{
}

void EffectKeyframe::setAddBlendEnable(bool value)
{
}

void EffectKeyframe::setShadowEnable(bool value)
{
}

void EffectKeyframe::setScaleFactor(float32 value)
{
}

void EffectKeyframe::setOpacity(float32 value)
{
}

void EffectKeyframe::setParentModelRef(IModel *value)
{
}

void EffectKeyframe::setParentBoneRef(IBone *value)
{
}

int EffectKeyframe::rowID() const
{
    return m_context->rowID;
}

void EffectKeyframe::setRowID(int value)
{
    m_context->rowID = value;
}

} /* namespace project */
