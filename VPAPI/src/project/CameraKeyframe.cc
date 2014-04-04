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
#include <project/CameraKeyframe.h>
#include <project/Motion.h>

using namespace vpvl2;

namespace project
{

struct CameraKeyframe::PrivateContext {
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

CameraKeyframe::CameraKeyframe(Motion *parent, int rowID)
    : m_context(new PrivateContext(parent, rowID))
{
}

CameraKeyframe::~CameraKeyframe()
{
    delete m_context;
    m_context = 0;
}

void CameraKeyframe::read(const uint8 *data)
{
}

void CameraKeyframe::write(uint8 *data) const
{
}

vsize CameraKeyframe::estimateSize() const
{
    return 0;
}

const IString *CameraKeyframe::name() const
{
    return 0;
}

IKeyframe::TimeIndex CameraKeyframe::timeIndex() const
{
    return 0;
}

IKeyframe::LayerIndex CameraKeyframe::layerIndex() const
{
    return 0;
}
void CameraKeyframe::setName(const IString *value)
{
}

void CameraKeyframe::setTimeIndex(const TimeIndex &value)
{
}

void CameraKeyframe::setLayerIndex(const LayerIndex &value)
{
}

IKeyframe::Type CameraKeyframe::type() const
{
    return kCameraKeyframe;
}

ICameraKeyframe *CameraKeyframe::clone() const
{
    return 0;
}

void CameraKeyframe::setDefaultInterpolationParameter()
{
}

void CameraKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
}

void CameraKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
}

Vector3 CameraKeyframe::lookAt() const
{
    return kZeroV3;
}

Vector3 CameraKeyframe::angle() const
{
    return kZeroV3;
}

Scalar CameraKeyframe::distance() const
{
    return 0;
}

Scalar CameraKeyframe::fov() const
{
    return 0;
}

bool CameraKeyframe::isPerspective() const
{
    return false;
}

void CameraKeyframe::setLookAt(const Vector3 &value)
{
}

void CameraKeyframe::setAngle(const Vector3 &value)
{
}

void CameraKeyframe::setDistance(const Scalar &value)
{
}

void CameraKeyframe::setFov(const Scalar &value)
{
}

void CameraKeyframe::setPerspective(bool value)
{
}

int CameraKeyframe::rowID() const
{
    return m_context->rowID;
}

void CameraKeyframe::setRowID(int value)
{
    m_context->rowID = value;
}

} /* namespace project */
