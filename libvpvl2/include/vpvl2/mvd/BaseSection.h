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

#ifndef VPVL2_MVD_BASESECTION_H_
#define VPVL2_MVD_BASESECTION_H_

#include "vpvl2/mvd/Motion.h"

namespace vpvl2
{
class IEncoding;

namespace mvd
{
class NameListSection;

class KeyframeTimeIndexPredication
{
public:
    bool operator()(const IKeyframe *left, const IKeyframe *right) const {
        return left->timeIndex() < right->timeIndex();
    }
};

struct BaseSectionContext {
    Array<IKeyframe *> *keyframes;
    mutable int lastIndex;
    BaseSectionContext()
        : keyframes(0),
          lastIndex(0)
    {
    }
    virtual ~BaseSectionContext() {
        if (keyframes)
            keyframes->releaseAll();
        lastIndex = 0;
    }
    void findKeyframeIndices(const IKeyframe::TimeIndex &seekIndex,
                             IKeyframe::TimeIndex &currentKeyframe,
                             int &fromIndex,
                             int &toIndex) const {
        const int nframes = keyframes->count();
        IKeyframe *lastKeyFrame = keyframes->at(nframes - 1);
        currentKeyframe = btMin(seekIndex, lastKeyFrame->timeIndex());
        // Find the next frame index bigger than the frame index of last key frame
        fromIndex = toIndex = 0;
        if (currentKeyframe >= keyframes->at(lastIndex)->timeIndex()) {
            for (int i = lastIndex; i < nframes; i++) {
                if (currentKeyframe <= keyframes->at(i)->timeIndex()) {
                    toIndex = i;
                    break;
                }
            }
        }
        else {
            for (int i = 0; i <= lastIndex && i < nframes; i++) {
                if (currentKeyframe <= keyframes->at(i)->timeIndex()) {
                    toIndex = i;
                    break;
                }
            }
        }
        if (toIndex >= nframes)
            toIndex = nframes - 1;
        fromIndex = toIndex <= 1 ? 0 : toIndex - 1;
        lastIndex = fromIndex;
    }
};

class VPVL2_API BaseSection
{
public:
    BaseSection(NameListSection *nameListSectionRef)
        : m_keyframeListPtr(0),
          m_nameListSectionRef(nameListSectionRef)
    {
    }
    virtual ~BaseSection() {
        release();
    }

    virtual void read(const uint8_t *data) = 0;
    virtual void write(uint8_t *data) const = 0;
    virtual size_t estimateSize() const = 0;
    virtual void release() {
        if (m_keyframeListPtr) {
            m_keyframeListPtr->releaseAll();
            delete m_keyframeListPtr;
            m_keyframeListPtr = 0;
        }
    }

protected:
    typedef Array<IKeyframe *> KeyframeList;
    KeyframeList *m_keyframeListPtr;
    NameListSection *m_nameListSectionRef;
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif

