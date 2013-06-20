/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

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
#ifndef VPVL2_VMD_CAMERAKEYFRAME_H_
#define VPVL2_VMD_CAMERAKEYFRAME_H_

#include "vpvl2/ICameraKeyframe.h"
#include "vpvl2/internal/Keyframe.h"

namespace vpvl2
{
namespace vmd
{

class VPVL2_API CameraKeyframe : public ICameraKeyframe
{
public:
    static vsize strideSize();

    CameraKeyframe();
    ~CameraKeyframe();

    static const int kTableSize = 24;
    static const QuadWord kDefaultInterpolationParameterValue;

    void read(const uint8 *data);
    void write(uint8 *data) const;
    vsize estimateSize() const;
    ICameraKeyframe *clone() const;

    void setName(const IString * /* value */) {}
    void setDefaultInterpolationParameter();
    void getInterpolationParameter(InterpolationType type, QuadWord &value) const;
    void setInterpolationParameter(InterpolationType type, const QuadWord &value);

    VPVL2_KEYFRAME_DEFINE_METHODS()
    Scalar distance() const { return m_distance; }
    Scalar fov() const { return m_fov; }
    Vector3 lookAt() const { return m_position; }
    Vector3 angle() const { return m_angle; }
    bool isPerspective() const { return !m_noPerspective; }
    const bool *linear() const { return m_linear; }
    const IKeyframe::SmoothPrecision *const *interpolationTable() const { return m_interpolationTable; }
    Type type() const { return IKeyframe::kCameraKeyframe; }

    void setDistance(const Scalar &value);
    void setFov(const Scalar &value);
    void setLookAt(const Vector3 &value);
    void setAngle(const Vector3 &value);
    void setPerspective(bool value);

private:
    void setInterpolationTable(const int8 *table);
    void setInterpolationParameterInternal(InterpolationType type, const QuadWord &value);
    QuadWord &getInterpolationParameterInternal(InterpolationType type) const;

    VPVL2_KEYFRAME_DEFINE_FIELDS()
    mutable CameraKeyframe *m_ptr;
    float m_distance;
    float m_fov;
    Vector3 m_position;
    Vector3 m_angle;
    bool m_noPerspective;
    bool m_linear[6];
    IKeyframe::SmoothPrecision *m_interpolationTable[6];
    int8 m_rawInterpolationTable[kTableSize];
    InterpolationParameter m_parameter;

    VPVL2_DISABLE_COPY_AND_ASSIGN(CameraKeyframe)
};

}
}

#endif
