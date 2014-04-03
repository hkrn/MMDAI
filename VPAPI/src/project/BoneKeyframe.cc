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
#include <project/BoneKeyframe.h>
#include <project/Motion.h>

using namespace vpvl2;

namespace project
{

struct BoneKeyframe::PrivateContext {
    PrivateContext(Motion *parent)
        : m_parentMotionRef(parent)
    {
    }
    ~PrivateContext() {
        m_parentMotionRef = 0;
    }

    Motion *m_parentMotionRef;
};

BoneKeyframe::BoneKeyframe(Motion *parent)
    : m_context(new PrivateContext(parent))
{
}

BoneKeyframe::~BoneKeyframe()
{
    delete m_context;
    m_context = 0;
}

void BoneKeyframe::read(const uint8 *data)
{
}

void BoneKeyframe::write(uint8 *data) const
{
}

vsize BoneKeyframe::estimateSize() const
{
    return 0;
}

const IString *BoneKeyframe::name() const
{
    return 0;
}

IKeyframe::TimeIndex BoneKeyframe::timeIndex() const
{
    return 0;
}

IKeyframe::LayerIndex BoneKeyframe::layerIndex() const
{
    return 0;
}
void BoneKeyframe::setName(const IString *value)
{
}

void BoneKeyframe::setTimeIndex(const TimeIndex &value)
{
}

void BoneKeyframe::setLayerIndex(const LayerIndex &value)
{
}

IKeyframe::Type BoneKeyframe::type() const
{
    return kBoneKeyframe;
}

IBoneKeyframe *BoneKeyframe::clone() const
{
    return 0;
}

void BoneKeyframe::setDefaultInterpolationParameter()
{
}

void BoneKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
}

void BoneKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
}

Vector3 BoneKeyframe::localTranslation() const
{
    return kZeroV3;
}

Quaternion BoneKeyframe::localOrientation() const
{
    return Quaternion::getIdentity();
}

void BoneKeyframe::setLocalTranslation(const Vector3 &value)
{
}

void BoneKeyframe::setLocalOrientation(const Quaternion &value)
{
}

} /* namespace project */
