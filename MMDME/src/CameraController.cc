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

#include "MMDME/MMDME.h"

namespace MMDAI {

CameraController::CameraController()
    : m_motion(0),
      m_distance(0),
      m_pos(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_fovy(0.0f),
      m_lastKey(0.0f),
      m_currentFrame(0),
      m_previousFrame(0)
{
}

CameraController::~CameraController()
{
    release();
}

void CameraController::release()
{
    m_motion = 0;
    m_distance = 0;
    m_fovy = 0.0f;
    m_lastKey = 0.0f;
    m_currentFrame = 0.0;
    m_previousFrame = 0.0;
    m_pos.setZero();
    m_angle.setZero();
}

void CameraController::control(float frameNow)
{
    float frame = frameNow;

    /* clamp frame to the defined last frame */
    if (frame > m_motion->keyFrameList[m_motion->numKeyFrame-1].keyFrame)
        frame = m_motion->keyFrameList[m_motion->numKeyFrame-1].keyFrame;

    /* find key frames between which the given frame exists */
    uint32_t k1 = 0, k2 = 0;
    if (frame >= m_motion->keyFrameList[m_lastKey].keyFrame) {
        /* start searching from last used key frame */
        for (uint32_t i = m_lastKey; i < m_motion->numKeyFrame; i++) {
            if (frame <= m_motion->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    } else {
        for (uint32_t i = 0; i <= m_lastKey && i < m_motion->numKeyFrame; i++) {
            if (frame <= m_motion->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    }

    /* bounding */
    if (k2 >= m_motion->numKeyFrame)
        k2 = m_motion->numKeyFrame - 1;
    if (k2 <= 1)
        k1 = 0;
    else
        k1 = k2 - 1;

    /* store the last key frame for next call */
    m_lastKey = k1;

    /* calculate the camera pameters at the specified frame */
    float time1 = m_motion->keyFrameList[k1].keyFrame;
    float time2 = m_motion->keyFrameList[k2].keyFrame;
    CameraKeyFrame *keyFrameForInterpolation = &(m_motion->keyFrameList[k2]);

    float distance1  = m_motion->keyFrameList[k1].distance;
    btVector3 pos1   = m_motion->keyFrameList[k1].pos;
    btVector3 angle1 = m_motion->keyFrameList[k1].angle;
    float fovy1      = m_motion->keyFrameList[k1].fovy;
    float distance2  = m_motion->keyFrameList[k2].distance;
    btVector3 pos2   = m_motion->keyFrameList[k2].pos;
    btVector3 angle2 = m_motion->keyFrameList[k2].angle;
    float fovy2      = m_motion->keyFrameList[k2].fovy;

    /* calculate the position and rotation */
    if (time1 != time2) {
        if (frame <= time1) {
            m_distance = distance1;
            m_pos      = pos1;
            m_angle    = angle1;
            m_fovy     = fovy1;
        } else if (frame >= time2) {
            m_distance = distance2;
            m_pos      = pos2;
            m_angle    = angle2;
            m_fovy     = fovy2;
        } else if (time2 - time1 <= 1.0f ) {
            /* successive keyframe in camera motion means camera switching, so do not perform interpolate between the frame */
            m_distance = distance1;
            m_pos      = pos1;
            m_angle    = angle1;
            m_fovy     = fovy1;
        } else {
            /* lerp */
            float x = 0.0f, y = 0.0f, z = 0.0f, ww = 0.0f;
            float w = (frame - time1) / (time2 - time1);
            int16_t idx = static_cast<int16_t>(w * VMD::kCameraInterpolationTableSize);
            if (keyFrameForInterpolation->linear[0]) {
                x = pos1.x() * (1.0f - w) + pos2.x() * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[0][idx] + (keyFrameForInterpolation->interpolationTable[0][idx+1] - keyFrameForInterpolation->interpolationTable[0][idx]) * (w * VMD::kCameraInterpolationTableSize - idx);
                x = pos1.x() * (1.0f - ww) + pos2.x() * ww;
            }
            if (keyFrameForInterpolation->linear[1]) {
                y = pos1.y() * (1.0f - w) + pos2.y() * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[1][idx] + (keyFrameForInterpolation->interpolationTable[1][idx+1] - keyFrameForInterpolation->interpolationTable[1][idx]) * (w * VMD::kCameraInterpolationTableSize - idx);
                y = pos1.y() * (1.0f - ww) + pos2.y() * ww;
            }
            if (keyFrameForInterpolation->linear[2]) {
                z = pos1.z() * (1.0f - w) + pos2.z() * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[2][idx] + (keyFrameForInterpolation->interpolationTable[2][idx+1] - keyFrameForInterpolation->interpolationTable[2][idx]) * (w * VMD::kCameraInterpolationTableSize - idx);
                z = pos1.z() * (1.0f - ww) + pos2.z() * ww;
            }
            m_pos.setValue(x, y, z);
            if (keyFrameForInterpolation->linear[3]) {
                m_angle = angle1.lerp(angle2, w);
            } else {
                ww = keyFrameForInterpolation->interpolationTable[3][idx] + (keyFrameForInterpolation->interpolationTable[3][idx+1] - keyFrameForInterpolation->interpolationTable[3][idx]) * (w * VMD::kCameraInterpolationTableSize - idx);
                m_angle = angle1.lerp(angle2, ww);
            }
            if (keyFrameForInterpolation->linear[4]) {
                m_distance = distance1 * (1.0f - w) + distance2 * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[4][idx] + (keyFrameForInterpolation->interpolationTable[4][idx+1] - keyFrameForInterpolation->interpolationTable[4][idx]) * (w * VMD::kCameraInterpolationTableSize - idx);
                m_distance = distance1 * (1.0f - ww) + distance2 * ww;
            }
            if (keyFrameForInterpolation->linear[5]) {
                m_fovy = fovy1 * (1.0f - w) + fovy2 * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[5][idx] + (keyFrameForInterpolation->interpolationTable[5][idx+1] - keyFrameForInterpolation->interpolationTable[5][idx]) * (w * VMD::kCameraInterpolationTableSize - idx);
                m_fovy = fovy1 * (1.0f - ww) + fovy2 * ww;
            }
        }
    } else {
        /* both keys have the same time, just apply one of them */
        m_distance = distance1;
        m_pos      = pos1;
        m_angle    = angle1;
        m_fovy     = fovy1;
    }
}

void CameraController::setup(VMD *vmd)
{
    release();
    m_motion = vmd->getCameraMotion();
}

void CameraController::reset()
{
    m_lastKey = 0;
    m_currentFrame = 0.0;
    m_previousFrame = 0.0;
}

bool CameraController::advance(double deltaFrame)
{
    if (!m_motion)
        return false;

    /* apply motion at current frame to bones and faces */
    control((float) m_currentFrame);

    /* advance the current frame count */
    /* store the last frame to m_previousFrame */
    m_previousFrame = m_currentFrame;
    m_currentFrame += deltaFrame;

    if (m_currentFrame >= m_motion->keyFrameList[m_motion->numKeyFrame-1].keyFrame) {
        /* we have reached the last key frame of this motion */
        /* clamp the frame to the maximum */
        m_currentFrame = m_motion->keyFrameList[m_motion->numKeyFrame-1].keyFrame;
        /* return finished status */
        return true;
    }

    return false;
}

} /* namespace */

