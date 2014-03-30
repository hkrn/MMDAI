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
#include <vpvl2/extensions/vpdb/ProjectKeyframe.h>

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace extensions
{
namespace vpdb
{

struct ProjectKeyframe::PrivateContext {
    PrivateContext(Motion *parent)
        : m_parentMotionRef(parent)
    {
    }
    ~PrivateContext() {
        m_parentMotionRef = 0;
    }

    Motion *m_parentMotionRef;
};

ProjectKeyframe::ProjectKeyframe(Motion *parent)
    : m_context(new PrivateContext(parent))
{
}

ProjectKeyframe::~ProjectKeyframe()
{
    delete m_context;
    m_context = 0;
}

void ProjectKeyframe::read(const uint8 *data)
{
}

void ProjectKeyframe::write(uint8 *data) const
{
}

vsize ProjectKeyframe::estimateSize() const
{
    return 0;
}

const IString *ProjectKeyframe::name() const
{
    return 0;
}

IKeyframe::TimeIndex ProjectKeyframe::timeIndex() const
{
    return 0;
}

IKeyframe::LayerIndex ProjectKeyframe::layerIndex() const
{
    return 0;
}
void ProjectKeyframe::setName(const IString *value)
{
}

void ProjectKeyframe::setTimeIndex(const TimeIndex &value)
{
}

void ProjectKeyframe::setLayerIndex(const LayerIndex &value)
{
}

IKeyframe::Type ProjectKeyframe::type() const
{
    return kProjectKeyframe;
}

IProjectKeyframe *ProjectKeyframe::clone() const
{
    return 0;
}

float32 ProjectKeyframe::gravityFactor() const
{
    return 0;
}

Vector3 ProjectKeyframe::gravityDirection() const
{
    kZeroV3;
}

int ProjectKeyframe::shadowMode() const
{
    return 0;
}

float32 ProjectKeyframe::shadowDistance() const
{
    return 0;
}

float32 ProjectKeyframe::shadowDepth() const
{
    return 0;
}

void ProjectKeyframe::setGravityFactor(float32 value)
{
}

void ProjectKeyframe::setGravityDirection(const Vector3 &value)
{
}

void ProjectKeyframe::setShadowMode(int value)
{
}

void ProjectKeyframe::setShadowDistance(float32 value)
{
}

void ProjectKeyframe::setShadowDepth(float32 value)
{
}

} /* namespace vpdb */
} /* namespace extensions */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */
