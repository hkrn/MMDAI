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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/mvd/CameraKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct CameraKeyframeChunk {
    int layerIndex;
    uint64_t timeIndex;
    float radius;
    float position[3];
    float rotation[3];
    float fov;
    uint8_t perspective;
    InterpolationPair positionIP;
    InterpolationPair rotationIP;
    InterpolationPair distanceIP;
    InterpolationPair fovIP;
};

#pragma pack(pop)

CameraKeyframe::CameraKeyframe(IEncoding *encoding)
    : BaseKeyframe(),
      m_encoding(encoding)
{
}

CameraKeyframe::~CameraKeyframe()
{
}

bool CameraKeyframe::preparse(const uint8_t *data, size_t &rest, Motion::DataInfo &info)
{
    return false;
}

void CameraKeyframe::read(const uint8_t *data)
{
}

void CameraKeyframe::write(uint8_t *data) const
{
}

size_t CameraKeyframe::estimateSize() const
{
    return 0;
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

const Vector3 &CameraKeyframe::position() const
{
    return kZeroV3;
}

const Vector3 &CameraKeyframe::angle() const
{
    return kZeroV3;
}

const Scalar &CameraKeyframe::distance() const
{
    return 0;
}

const Scalar &CameraKeyframe::fov() const
{
    return 0;
}

bool CameraKeyframe::isPerspective() const
{
    return false;
}

void CameraKeyframe::setPosition(const Vector3 &value)
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

}
}
