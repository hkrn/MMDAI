/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#ifndef VPVL2_VMD_BASEANIMATION_H_
#define VPVL2_VMD_BASEANIMATION_H_

#include "vpvl2/Common.h"
#include "vpvl2/IKeyframe.h"

namespace vpvl2
{
namespace vmd
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * The basic class inherited by BoneAnimation, CameraAnimation and FaceAnimation class.
 */

class VPVL2_API BaseAnimation
{
public:

    BaseAnimation();
    virtual ~BaseAnimation();

    virtual void read(const uint8_t *data, int size) = 0;
    virtual void seek(float frameAt) = 0;
    void advance(float deltaFrame);
    void rewind(float target, float deltaFrame);
    void reset();
    void addKeyframe(IKeyframe *frame);
    void deleteKeyframe(IKeyframe *&frame);
    void deleteKeyframes(int frameIndex);

    int countKeyframes() const { return m_keyframes.count(); }
    float previousIndex() const { return m_previousFrameIndex; }
    float currentIndex() const { return m_currentFrameIndex; }
    float maxIndex() const { return m_maxFrameIndex; }

protected:
    template<typename T>
    static int findKeyframeIndex(int key, const Array<T *> &keyframes) {
        int first = 0, size = keyframes.count(), last = size;
        if (size == 0)
            return -1;
        while (first < last) {
            int mid = (first + last) / 2;
            const T *keyframe = keyframes[mid];
            const int frameIndex = int(keyframe->frameIndex());
            if (mid >= size)
                return -1;
            if (key > frameIndex)
                first = mid + 1;
            else if (key < frameIndex)
                last = mid - 1;
            else
                return mid;
        }
        return -1;
    }

    Array<IKeyframe *> m_keyframes;
    int m_lastIndex;
    float m_maxFrameIndex;
    float m_currentFrameIndex;
    float m_previousFrameIndex;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseAnimation)
};

}
}

#endif

