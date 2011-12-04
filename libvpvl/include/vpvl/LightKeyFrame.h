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

#ifndef VPVL_LIGHTKEYFRAME_H_
#define VPVL_LIGHTKEYFRAME_H_

#include "vpvl/BaseKeyFrame.h"

namespace vpvl
{

class VPVL_API LightKeyFrame : public BaseKeyFrame
{
public:
    LightKeyFrame();
    ~LightKeyFrame();

    static size_t strideSize();

    size_t stride() const;
    void read(const uint8_t *data);
    void write(uint8_t *data) const;
    BaseKeyFrame *clone() const;

    /**
     * Returns empty value.
     *
     * @return null name
     */
    const uint8_t *name() const;

    /**
     * Returns light color of this keyframe.
     *
     * @return A value of light color
     */
    Vector3 color() const {
        return m_color;
    }

    /**
     * Returns light direction of this keyframe.
     *
     * @return A value of light direction
     */
    Vector3 direction() const {
        return m_direction;
    }

    /**
     * Set the name of this keyframe.
     *
     * This method do nothing.
     *
     * @param value A value of name
     */
    void setName(const uint8_t *value);

    /**
     * Set light color of this keyframe.
     *
     * @param A value of light color
     */
    void setColor(const Vector3 &value) {
        m_color = value;
    }

    /**
     * Set light direction of this keyframe.
     *
     * @param A value of light direction
     */
    void setDirection(const Vector3 &value) {
        m_direction = value;
    }

private:
    uint8_t m_name[2];
    Vector3 m_color;
    Vector3 m_direction;

    VPVL_DISABLE_COPY_AND_ASSIGN(LightKeyFrame)
};

}

#endif
