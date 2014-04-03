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
#include <project/MorphKeyframe.h>
#include <project/Motion.h>

using namespace vpvl2;

namespace project
{

struct MorphKeyframe::PrivateContext {
    PrivateContext(Motion *parent)
        : m_parentMotionRef(parent)
    {
    }
    ~PrivateContext() {
        m_parentMotionRef = 0;
    }

    Motion *m_parentMotionRef;
};

MorphKeyframe::MorphKeyframe(Motion *parent)
    : m_context(new PrivateContext(parent))
{
}

MorphKeyframe::~MorphKeyframe()
{
    delete m_context;
    m_context = 0;
}

void MorphKeyframe::read(const uint8 *data)
{
}

void MorphKeyframe::write(uint8 *data) const
{
}

vsize MorphKeyframe::estimateSize() const
{
    return 0;
}

const IString *MorphKeyframe::name() const
{
    return 0;
}

IKeyframe::TimeIndex MorphKeyframe::timeIndex() const
{
    return 0;
}

IKeyframe::LayerIndex MorphKeyframe::layerIndex() const
{
    return 0;
}
void MorphKeyframe::setName(const IString *value)
{
}

void MorphKeyframe::setTimeIndex(const TimeIndex &value)
{
}

void MorphKeyframe::setLayerIndex(const LayerIndex &value)
{
}

IKeyframe::Type MorphKeyframe::type() const
{
    return kMorphKeyframe;
}

IMorphKeyframe *MorphKeyframe::clone() const
{
    return 0;
}

IMorph::WeightPrecision MorphKeyframe::weight() const
{
    return 0;
}

void MorphKeyframe::setWeight(const IMorph::WeightPrecision &value)
{
}

} /* namespace project */
