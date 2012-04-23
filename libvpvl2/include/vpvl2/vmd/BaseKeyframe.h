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

#ifndef VPVL2_VMD_BASEKEYFRAME_H_
#define VPVL2_VMD_BASEKEYFRAME_H_

#include "vpvl2/Common.h"
#include "vpvl2/IKeyframe.h"

namespace vpvl2
{
namespace vmd
{

class VPVL2_API BaseKeyframe : public virtual IKeyframe
{
public:
    BaseKeyframe()
        : m_name(0),
          m_frameIndex(0)
    {
    }
    virtual ~BaseKeyframe() {
        delete m_name;
        m_name = 0;
        m_frameIndex = 0;
    }

    /**
     * Get the target bone name of this keyframe.
     *
     * @return name the bone name
     */
    const IString *name() const { return m_name; }

    /**
     * Get the frame index of this keyframe.
     *
     * @return A value of frame index
     */
    float frameIndex() const { return m_frameIndex; }

    /**
     * Set the frame index of this keyframe.
     *
     * @param value A value of frame index
     */
    void setFrameIndex(float value) { m_frameIndex = value; }

protected:
    IString *m_name;
    float m_frameIndex;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseKeyframe)
};

}
}

#endif
