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
#include <vpvl2/extensions/vpdb/ModelKeyframe.h>
#include <vpvl2/extensions/vpdb/Motion.h>

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace extensions
{
namespace vpdb
{

struct ModelKeyframe::PrivateContext {
    PrivateContext(Motion *parent)
        : m_parentMotionRef(parent)
    {
    }
    ~PrivateContext() {
        m_parentMotionRef = 0;
    }

    Motion *m_parentMotionRef;
};

ModelKeyframe::ModelKeyframe(Motion *parent)
    : m_context(new PrivateContext(parent))
{
}

ModelKeyframe::~ModelKeyframe()
{
    delete m_context;
    m_context = 0;
}

void ModelKeyframe::read(const uint8 *data)
{
}

void ModelKeyframe::write(uint8 *data) const
{
}

vsize ModelKeyframe::estimateSize() const
{
    return 0;
}

const IString *ModelKeyframe::name() const
{
    return 0;
}

IKeyframe::TimeIndex ModelKeyframe::timeIndex() const
{
    return 0;
}

IKeyframe::LayerIndex ModelKeyframe::layerIndex() const
{
    return 0;
}
void ModelKeyframe::setName(const IString *value)
{
}

void ModelKeyframe::setTimeIndex(const TimeIndex &value)
{
}

void ModelKeyframe::setLayerIndex(const LayerIndex &value)
{
}

IKeyframe::Type ModelKeyframe::type() const
{
    return kModelKeyframe;
}

IModelKeyframe *ModelKeyframe::clone() const
{
    return 0;
}

bool ModelKeyframe::isVisible() const
{
    return false;
}

bool ModelKeyframe::isShadowEnabled() const
{
    return false;
}

bool ModelKeyframe::isAddBlendEnabled() const
{
    return false;
}

bool ModelKeyframe::isPhysicsEnabled() const
{
    return false;
}

bool ModelKeyframe::isInverseKinematicsEnabled(const IBone *value) const
{
    return false;
}

uint8 ModelKeyframe::physicsStillMode() const
{
    return 0;
}

IVertex::EdgeSizePrecision ModelKeyframe::edgeWidth() const
{
    return 0;
}

Color ModelKeyframe::edgeColor() const
{
    return kZeroC;
}

void ModelKeyframe::setVisible(bool value)
{
}

void ModelKeyframe::setShadowEnable(bool value)
{
}

void ModelKeyframe::setAddBlendEnable(bool value)
{
}

void ModelKeyframe::setPhysicsEnable(bool value)
{
}

void ModelKeyframe::setPhysicsStillMode(uint8 value)
{
}

void ModelKeyframe::setEdgeWidth(const IVertex::EdgeSizePrecision &value)
{
}

void ModelKeyframe::setEdgeColor(const Color &value)
{
}

void ModelKeyframe::setInverseKinematicsEnable(IBone *bone, bool value)
{
}

} /* namespace vpdb */
} /* namespace extensions */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */
