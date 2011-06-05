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

#ifndef VPVL_BASEMOTION_H_
#define VPVL_BASEMOTION_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"

namespace vpvl
{

class BaseMotion
{
public:
    BaseMotion(float smearDefault);
    virtual ~BaseMotion() {}

    virtual void read(const char *data, uint32_t size) = 0;
    virtual void seek(float frameAt) = 0;
    virtual void takeSnap(const btVector3 &center) = 0;

    void advance(float deltaFrame, bool &reached);
    void rewind(float target, float frameAt);
    void reset();
    void setOverrideFirst(const btVector3 &center);

    float blendRate() const {
        return m_blendRate;
    }
    float maxIndex() const {
        return m_maxFrame;
    }
    void setBlendRate(float value) {
        m_blendRate = value;
    }

protected:
    uint32_t m_lastIndex;
    const float m_smearDefault;
    float m_maxFrame;
    float m_currentFrame;
    float m_previousFrame;
    float m_lastLoopStartFrame;
    float m_blendRate;
    float m_smearIndex;
    bool m_ignoreSingleMotion;
    bool m_overrideFirst;
};

}

#endif

