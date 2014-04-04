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
#include <project/LightKeyframe.h>

using namespace vpvl2;

namespace project
{

struct LightKeyframe::PrivateContext {
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

LightKeyframe::LightKeyframe(Motion *parent, int rowID)
    : m_context(new PrivateContext(parent, rowID))
{
}

LightKeyframe::~LightKeyframe()
{
    delete m_context;
    m_context = 0;
}

void LightKeyframe::read(const uint8 *data)
{
}

void LightKeyframe::write(uint8 *data) const
{
}

vsize LightKeyframe::estimateSize() const
{
    return 0;
}

const IString *LightKeyframe::name() const
{
    return 0;
}

IKeyframe::TimeIndex LightKeyframe::timeIndex() const
{
    return 0;
}

IKeyframe::LayerIndex LightKeyframe::layerIndex() const
{
    return 0;
}
void LightKeyframe::setName(const IString *value)
{
}

void LightKeyframe::setTimeIndex(const TimeIndex &value)
{
}

void LightKeyframe::setLayerIndex(const LayerIndex &value)
{
}

IKeyframe::Type LightKeyframe::type() const
{
    return kLightKeyframe;
}

ILightKeyframe *LightKeyframe::clone() const
{
    return 0;
}

Vector3 LightKeyframe::color() const
{
    return kZeroV3;
}

Vector3 LightKeyframe::direction() const
{
    return kZeroV3;
}

void LightKeyframe::setColor(const Vector3 &value)
{
}

void LightKeyframe::setDirection(const Vector3 &value)
{
}

int LightKeyframe::rowID() const
{
    return m_context->rowID;
}

void LightKeyframe::setRowID(int value)
{
    m_context->rowID = value;
}

} /* namespace project */
