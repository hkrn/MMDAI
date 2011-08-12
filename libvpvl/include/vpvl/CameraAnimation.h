/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_CAMERAANIMATION_H_
#define VPVL_CAMERAANIMATION_H_

#include "vpvl/BaseAnimation.h"

namespace vpvl
{

class CameraKeyFrame;
typedef btAlignedObjectArray<CameraKeyFrame *> CameraKeyFrameList;

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

class VPVL_EXPORT CameraAnimation : public BaseAnimation
{
public:
    CameraAnimation();
    ~CameraAnimation();

    void read(const uint8_t *data, uint32_t size);
    void seek(float frameAt);
    void takeSnap(const btVector3 &center);
    void reset();
    void refresh();

    /**
     * Get a camera key frame associated with index.
     *
     * @param A frame index to get key frame
     * @return A camera key frame associated with index
     */
    CameraKeyFrame *frameAt(uint32_t i) const {
        return static_cast<CameraKeyFrame *>(m_frames[i]);
    }

    const btVector3 &position() const {
        return m_position;
    }
    const btVector3 &angle() const {
        return m_angle;
    }
    float distance() const {
        return m_distance;
    }
    float fovy() const {
        return m_fovy;
    }

private:
    static float weightValue(const CameraKeyFrame *keyFrame,
                             float w,
                             uint32_t at);
    static void lerpVector3(const CameraKeyFrame *keyFrame,
                            const btVector3 &from,
                            const btVector3 &to,
                            float w,
                            uint32_t at,
                            float &value);

    btVector3 m_position;
    btVector3 m_angle;
    float m_distance;
    float m_fovy;
    uint32_t m_lastIndex;

    VPVL_DISABLE_COPY_AND_ASSIGN(CameraAnimation)
};

}

#endif

