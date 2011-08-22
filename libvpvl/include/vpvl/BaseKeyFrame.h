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

#ifndef VPVL_BASEKEYFRAME_H_
#define VPVL_BASEKEYFRAME_H_

#include "vpvl/Common.h"

namespace vpvl
{

class VPVL_EXPORT BaseKeyFrame
{
public:
    BaseKeyFrame() : m_frameIndex(0) {
    }
    virtual ~BaseKeyFrame() {
        m_frameIndex = 0;
    }

    /**
     * Stride length of a keyframe structure.
     *
     * @return Stride length
     */
    virtual size_t stride() const = 0;

    /**
     * Read and parse the buffer and sets it's result to the class.
     *
     * @param The buffer to read and parse
     */
    virtual void read(const uint8_t *data) = 0;

    /**
     * Write the current value to the buffer.
     *
     * You should allocate the buffer size with stride.
     *
     * @param The buffer to write
     * @see stride
     */
    virtual void write(uint8_t *data) const = 0;

    /**
     * Clone a key frame
     *
     * You must manage a copied keyframe and free it at destruction.
     *
     * @return copied key frame
     */
    virtual BaseKeyFrame *clone() const = 0;

    /**
     * Get the target bone name of this keyframe.
     *
     * @return the bone name
     */
    virtual const uint8_t *name() const = 0;

    /**
     * Get the frame index of this keyframe.
     *
     * @return A value of frame index
     */
    float frameIndex() const {
        return m_frameIndex;
    }

    /**
     * Set the target bone name of this keyframe.
     *
     * @param the bone name
     */
    virtual void setName(const uint8_t *value) = 0;

    /**
     * Set the frame index of this keyframe.
     *
     * @param A value of frame index
     */
    void setFrameIndex(float value) {
        m_frameIndex = value;
    }

protected:
    float m_frameIndex;

    VPVL_DISABLE_COPY_AND_ASSIGN(BaseKeyFrame)
};

}

#endif

