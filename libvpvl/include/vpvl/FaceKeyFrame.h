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

#ifndef VPVL_FACEKEYFRAME_H_
#define VPVL_FACEKEYFRAME_H_

#include <LinearMath/btAlignedObjectArray.h>
#include "vpvl/BaseKeyFrame.h"

namespace vpvl
{

class Face;

class VPVL_EXPORT FaceKeyFrame : public BaseKeyFrame
{
public:
    FaceKeyFrame();
    ~FaceKeyFrame();

    static const int kNameSize = 15;

    static size_t strideSize();

    size_t stride() const;
    void read(const uint8_t *data);
    void write(uint8_t *data) const;

    /**
     * Get the target bone name of this keyframe.
     *
     * @return the bone name
     */
    const uint8_t *name() const;

    /**
     * Get the face weight of this keyframe.
     *
     * @return A value of face weight
     */
    float weight() const {
        return m_weight;
    }

    /**
     * Set the target bone name of this keyframe.
     *
     * @param the bone name
     */
    void setName(const uint8_t *value);

    /**
     * Set the face weight of this keyframe.
     *
     * @param A value of face weight
     */
    void setWeight(float value) {
        m_weight = value;
    }

private:
    uint8_t m_name[kNameSize];
    float m_weight;

    VPVL_DISABLE_COPY_AND_ASSIGN(FaceKeyFrame)
};

}

#endif
