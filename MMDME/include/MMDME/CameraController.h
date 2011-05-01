/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#ifndef MMDAI_CAMERACONTROLLER_H_
#define MMDAI_CAMERACONTROLLER_H_

#include "MMDME/Common.h"
#include "MMDME/VMD.h"

namespace MMDAI {

class CameraController
{
public:
    CameraController();
    ~CameraController();

    void setup(VMD *motion);
    void reset();
    bool advance(double deltaFrame);

    void getCurrentViewParam(float *distance, btVector3 *pos, btVector3 *angle, float *fovy) {
       *distance = m_distance;
       *pos = m_pos;
       *angle = m_angle;
       *fovy = m_fovy;
    }
    inline double getCurrentFrame() {
       return m_currentFrame;
    }
    inline void setCurrentFrame(double frame) {
       m_currentFrame = frame;
    }
    inline double getPreviousFrame() {
       return m_previousFrame;
    }
    inline void setPreviousFrame(double frame) {
       m_previousFrame = frame;
    }

private:
    void release();
    void control(float frameNow);

    CameraMotion *m_motion;
    float m_distance;
    btVector3 m_pos;
    btVector3 m_angle;
    float m_fovy;
    uint32_t m_lastKey;

    double m_currentFrame;
    double m_previousFrame;
};

} /* namespace */

#endif
