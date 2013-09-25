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
#ifndef VPVL2_VMD_CAMERAANIMATION_H_
#define VPVL2_VMD_CAMERAANIMATION_H_

#include "vpvl2/vmd/BaseAnimation.h"

namespace vpvl2
{
namespace vmd
{

class CameraKeyframe;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * CameraAnimation class represents a camera Animation that includes many camera key frames
 * of a Vocaloid Motion Data object inherits BaseAnimation.
 */

class VPVL2_API CameraAnimation VPVL2_DECL_FINAL : public BaseAnimation
{
public:
    CameraAnimation();
    ~CameraAnimation();

    void read(const uint8 *data, int size);
    void seek(const IKeyframe::TimeIndex &timeIndexAt);
    void update();
    CameraKeyframe *findKeyframe(const IKeyframe::TimeIndex &timeIndex) const;
    CameraKeyframe *findKeyframeAt(int i) const;

    Vector3 position() const { return m_position; }
    Vector3 angle() const { return m_angle; }
    Scalar distance() const { return m_distance; }
    Scalar fovy() const { return m_fovy; }

private:
    static IKeyframe::SmoothPrecision weightValue(const CameraKeyframe *keyFrame,
                                                  const IKeyframe::SmoothPrecision &w,
                                                  int at);
    static void lerpVector3(const CameraKeyframe *keyframe,
                            const Vector3 &from,
                            const Vector3 &to,
                            const IKeyframe::SmoothPrecision &w,
                            int at,
                            IKeyframe::SmoothPrecision &value);

    Vector3 m_position;
    Vector3 m_angle;
    Scalar m_distance;
    Scalar m_fovy;

    VPVL2_DISABLE_COPY_AND_ASSIGN(CameraAnimation)
};

}
}

#endif

