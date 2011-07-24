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

#include "vpvl/vpvl.h"

namespace vpvl
{

class CameraMotionKeyFramePredication
{
public:
    bool operator()(const CameraKeyFrame *left, const CameraKeyFrame *right) {
        return left->frameIndex() < right->frameIndex();
    }
};

float CameraMotion::weightValue(const CameraKeyFrame *keyFrame, float w, uint32_t at)
{
    const uint16_t index = static_cast<int16_t>(w * CameraKeyFrame::kTableSize);
    const float *v = keyFrame->interpolationTable()[at];
    return v[index] + (v[index + 1] - v[index]) * (w * CameraKeyFrame::kTableSize - index);
}

void CameraMotion::lerpVector3(const CameraKeyFrame *keyFrame,
                               const btVector3 &from,
                               const btVector3 &to,
                               float w,
                               uint32_t at,
                               float &value)
{
    const float valueFrom = static_cast<const btScalar *>(from)[at];
    const float valueTo = static_cast<const btScalar *>(to)[at];
    if (keyFrame->linear()[at]) {
        value = valueFrom * (1.0f - w) + valueTo * w;
    }
    else {
        const float w2 = weightValue(keyFrame, w, at);
        value = valueFrom * (1.0f - w2) + valueTo * w2;
    }
}

CameraMotion::CameraMotion()
    : BaseMotion(0.0f),
      m_position(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_distance(0.0f),
      m_fovy(0.0f),
      m_lastIndex(0)
{
}

CameraMotion::~CameraMotion()
{
    internal::clearAll(m_frames);
    m_position.setZero();
    m_angle.setZero();
    m_distance = 0.0f;
    m_fovy = 0.0f;
    m_lastIndex = 0;
}

void CameraMotion::read(const uint8_t *data, uint32_t size)
{
    if (size > 0) {
        uint8_t *ptr = const_cast<uint8_t *>(data);
        m_frames.reserve(size);
        for (uint32_t i = 0; i < size; i++) {
            CameraKeyFrame *frame = new CameraKeyFrame();
            frame->read(ptr);
            ptr += CameraKeyFrame::stride();
            m_frames.push_back(frame);
        }
        m_frames.quickSort(CameraMotionKeyFramePredication());
        m_maxFrame = m_frames[size - 1]->frameIndex();
    }
}

void CameraMotion::seek(float frameAt)
{
    const uint32_t nFrames = m_frames.size();
    CameraKeyFrame *lastKeyFrame = m_frames[nFrames - 1];
    float currentFrame = frameAt;
    if (currentFrame > lastKeyFrame->frameIndex())
        currentFrame = lastKeyFrame->frameIndex();

    uint32_t k1 = 0, k2 = 0;
    if (currentFrame >= m_frames[m_lastIndex]->frameIndex()) {
        for (uint32_t i = m_lastIndex; i < nFrames; i++) {
            if (currentFrame <= m_frames[i]->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (uint32_t i = 0; i <= m_lastIndex && i < nFrames; i++) {
            if (currentFrame <= m_frames[i]->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nFrames)
        k2 = nFrames - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    m_lastIndex = k1;

    const CameraKeyFrame *keyFrameFrom = m_frames.at(k1), *keyFrameTo = m_frames.at(k2);
    CameraKeyFrame *keyFrameForInterpolation = const_cast<CameraKeyFrame *>(keyFrameTo);
    float frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    float distanceFrom = keyFrameFrom->distance(), fovyFrom = keyFrameFrom->fovy();
    btVector3 positionFrom = keyFrameFrom->position(), angleFrom = keyFrameFrom->angle();
    float distanceTo = keyFrameTo->distance(), fovyTo = keyFrameTo->fovy();
    btVector3 positionTo = keyFrameTo->position(), angleTo = keyFrameTo->angle();
    if (frameIndexFrom != frameIndexTo) {
        if (currentFrame <= frameIndexFrom) {
            m_distance = distanceFrom;
            m_position = positionFrom;
            m_angle = angleFrom;
            m_fovy = fovyFrom;
        }
        else if (currentFrame >= frameIndexTo) {
            m_distance = distanceTo;
            m_position = positionTo;
            m_angle = angleTo;
            m_fovy = fovyTo;
        }
        else if (frameIndexTo - frameIndexFrom <= 1.0f) {
            m_distance = distanceFrom;
            m_position = positionFrom;
            m_angle = angleFrom;
            m_fovy = fovyFrom;
        }
        else {
            const float w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
            float x = 0, y = 0, z = 0;
            lerpVector3(keyFrameForInterpolation, positionFrom, positionTo, w, 0, x);
            lerpVector3(keyFrameForInterpolation, positionFrom, positionTo, w, 1, y);
            lerpVector3(keyFrameForInterpolation, positionFrom, positionTo, w, 2, z);
            m_position.setValue(x, y, z);
            if (keyFrameForInterpolation->linear()[3]) {
                m_angle = angleFrom.lerp(angleTo, w);
            }
            else {
                const float w2 = weightValue(keyFrameForInterpolation, w, 3);
                m_angle = angleFrom.lerp(angleTo, w2);
            }
            if (keyFrameForInterpolation->linear()[4]) {
                m_distance = internal::lerp(distanceFrom, distanceTo, w);
            }
            else {
                const float w2 = weightValue(keyFrameForInterpolation, w, 4);
                m_distance = internal::lerp(distanceFrom, distanceTo, w2);
            }
            if (keyFrameForInterpolation->linear()[5]) {
                m_fovy = internal::lerp(fovyFrom, fovyTo, w);
            }
            else {
                const float w2 = weightValue(keyFrameForInterpolation, w, 5);
                m_fovy = internal::lerp(fovyFrom, fovyTo, w2);
            }
        }
    }
    else {
        m_distance = distanceFrom;
        m_position = positionFrom;
        m_angle = angleFrom;
        m_fovy = fovyFrom;
    }
}

void CameraMotion::takeSnap(const btVector3 & /* center */)
{
}

void CameraMotion::reset()
{
    BaseMotion::reset();
}

}
