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
#include "vpvl/common.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

class Face;

class FaceKeyFrame
{
public:
    FaceKeyFrame() : m_frameIndex(0), m_weight(0.0f) {
        internal::zerofill(m_name, sizeof(m_name));
    }
    ~FaceKeyFrame() {
        internal::zerofill(m_name, sizeof(m_name));
    }

    static const int kNameSize = 15;

    static size_t stride() {
        return kNameSize + sizeof(uint32_t) + sizeof(float);
    }

    void read(const uint8_t *data) {
        uint8_t *ptr = const_cast<uint8_t *>(data);
        copyBytesSafe(m_name, ptr, sizeof(m_name));
        ptr += sizeof(m_name);
        uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        float weight = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);

        m_frameIndex = static_cast<float>(index);
        m_weight = weight;
    }

    const uint8_t *name() const {
        return m_name;
    }
    float frameIndex() const {
        return m_frameIndex;
    }
    float weight() const {
        return m_weight;
    }

private:
    uint8_t m_name[kNameSize];
    float m_frameIndex;
    float m_weight;

    VPVL_DISABLE_COPY_AND_ASSIGN(FaceKeyFrame)
};

}

#endif
