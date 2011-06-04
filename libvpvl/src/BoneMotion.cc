/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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
#include "vpvl/internal/BoneKeyFrame.h"

namespace vpvl
{

const float BoneMotion::kStartingMarginFrame = 20.0f;

class BoneMotionKeyFramePredication
{
public:
    bool operator()(const BoneKeyFrame *left, const BoneKeyFrame *right) {
        return left->index() - right->index();
    }
};

BoneMotion::BoneMotion()
    : m_bone(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_snapPosition(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_snapRotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_lastIndex(0),
      m_lastLoopStartIndex(0),
      m_noBoneSmearIndex(kStartingMarginFrame),
      m_overrideFirst(false)
{
}

BoneMotion::~BoneMotion()
{
    m_position.setZero();
    m_snapPosition.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_snapRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_lastIndex = 0;
    m_lastLoopStartIndex = 0;
    m_noBoneSmearIndex = 0;
    m_overrideFirst = false;
}

void BoneMotion::read(const char *data, uint32_t size)
{
    char *ptr = const_cast<char *>(data);
    m_frames.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        BoneKeyFrame *frame = new BoneKeyFrame();
        frame->read(ptr);
        ptr += BoneKeyFrame::stride(ptr);
        m_frames.push_back(frame);
    }
}

void BoneMotion::calculate(float frameAt)
{
    uint32_t nFrames = m_frames.size();
    BoneKeyFrame *lastKeyFrame = m_frames.at(nFrames - 1);
    float currentFrame = frameAt;
    if (currentFrame > lastKeyFrame->index())
        currentFrame = lastKeyFrame->index();

    uint32_t k1 = 0, k2 = 0;
    if (currentFrame >= m_frames.at(m_lastIndex)->index()) {
        for (uint32_t i = m_lastIndex; i < nFrames; i++) {
            if (currentFrame <= m_frames.at(i)->index()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (uint32_t i = 0; i <= m_lastIndex && i < nFrames; i++) {
            if (currentFrame <= m_frames.at(i)->index()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nFrames)
        k2 = nFrames - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    m_lastIndex = k1;

    const BoneKeyFrame *keyFrameFrom = m_frames.at(k1), *keyFrameTo = m_frames.at(k2);
    float timeFrom = keyFrameFrom->index();
    float timeTo = keyFrameTo->index();
    BoneKeyFrame *keyFrameForInterpolation = const_cast<BoneKeyFrame *>(keyFrameTo);
    btVector3 positionFrom(0.0f, 0.0f, 0.0f), positionTo(0.0f, 0.0f, 0.0f);
    btQuaternion rotationFrom(0.0f, 0.0f, 0.0f, 1.0f), rotationTo(0.0f, 0.0f, 0.0f, 1.0f);
    if (m_overrideFirst &&(k1 == 0 || timeFrom <= m_lastIndex <= m_lastLoopStartIndex)) {
        if (nFrames > 1 && timeTo < m_lastLoopStartIndex + 60.0f) {
            timeFrom = m_lastLoopStartIndex;
            positionFrom = m_snapPosition;
            rotationFrom = m_snapRotation;
            positionTo = keyFrameTo->position();
            rotationTo = keyFrameTo->rotation();
        }
        else if (frameAt - timeFrom < m_noBoneSmearIndex) {
            timeFrom = m_lastLoopStartIndex;
            timeTo = m_lastLoopStartIndex + m_noBoneSmearIndex;
            currentFrame = frameAt;
            positionFrom = m_snapPosition;
            rotationFrom = m_snapRotation;
            positionTo = keyFrameFrom->position();
            rotationTo = keyFrameFrom->rotation();
            keyFrameForInterpolation = const_cast<BoneKeyFrame *>(keyFrameFrom);
        }
        else if (nFrames > 1) {
            timeFrom = m_lastLoopStartIndex + m_noBoneSmearIndex;
            currentFrame = frameAt;
            positionFrom = keyFrameFrom->position();
            rotationFrom = keyFrameFrom->rotation();
            positionTo = keyFrameTo->position();
            rotationTo = keyFrameTo->rotation();
        }
        else {
            positionFrom = keyFrameFrom->position();
            rotationFrom = keyFrameFrom->rotation();
        }
    }
    else {
        positionFrom = keyFrameFrom->position();
        rotationFrom = keyFrameFrom->rotation();
        positionTo = keyFrameTo->position();
        rotationTo = keyFrameTo->rotation();
    }

    if (timeFrom != timeTo) {
        if (currentFrame <= timeFrom) {
            m_position = positionFrom;
            m_rotation = rotationFrom;
        }
        else if (currentFrame >= timeTo) {
            m_position = positionTo;
            m_rotation = rotationTo;
        }
        else {
            float w = (currentFrame - timeFrom) / (timeTo - timeFrom);
            float x = 0, y = 0, z = 0;
            lerpPosition(keyFrameForInterpolation, positionFrom, positionTo, w, 0, x);
            lerpPosition(keyFrameForInterpolation, positionFrom, positionTo, w, 1, y);
            lerpPosition(keyFrameForInterpolation, positionFrom, positionTo, w, 2, z);
            m_position.setValue(x, y, z);
            if (keyFrameForInterpolation->linear()[3]) {
                m_rotation = rotationFrom.slerp(rotationTo, w);
            }
            else {
                const float *v = keyFrameForInterpolation->interpolationTable()[3];
                const int16_t index = static_cast<int16_t>(w * BoneKeyFrame::kTableSize);
                const float w2 = v[index] + (v[index + 1] - v[index]) * (w * BoneKeyFrame::kTableSize - index);
                m_rotation = rotationFrom.slerp(rotationTo, w2);
            }
        }
    }
    else {
        m_position = positionFrom;
        m_rotation = rotationFrom;
    }
}

void BoneMotion::sort()
{
    m_frames.quickSort(BoneMotionKeyFramePredication());
}

void BoneMotion::lerpPosition(const BoneKeyFrame *keyFrame,
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
        const uint16_t index = static_cast<int16_t>(w * BoneKeyFrame::kTableSize);
        const float *v = keyFrame->interpolationTable()[at];
        const float w2 = v[index] + (v[index + 1] - v[index]) * (w * BoneKeyFrame::kTableSize - index);
        value = valueFrom * (1.0f - w2) + valueTo * w2;
    }
}

}
