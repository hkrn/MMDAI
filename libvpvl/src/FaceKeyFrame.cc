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
#include "vpvl/internal/util.h"

namespace vpvl
{

#pragma pack(push, 1)

struct FaceKeyFrameChunk
{
    uint8_t name[15];
    uint32_t frameIndex;
    float weight;
};

#pragma pack(pop)

FaceKeyFrame::FaceKeyFrame()
    : BaseKeyFrame(), m_weight(0.0f)
{
    internal::zerofill(m_name, sizeof(m_name));
}
FaceKeyFrame::~FaceKeyFrame()
{
    internal::zerofill(m_name, sizeof(m_name));
}

size_t FaceKeyFrame::strideSize()
{
    return sizeof(FaceKeyFrameChunk);
}

size_t FaceKeyFrame::stride() const
{
    return strideSize();
}

void FaceKeyFrame::read(const uint8_t *data)
{
    FaceKeyFrameChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    copyBytesSafe(m_name, chunk.name, sizeof(m_name));
    m_frameIndex = static_cast<float>(chunk.frameIndex);
    m_weight = chunk.weight;
}

void FaceKeyFrame::write(uint8_t *data) const
{
    FaceKeyFrameChunk chunk;
    copyBytesSafe(chunk.name, m_name, sizeof(chunk.name));
    chunk.frameIndex = static_cast<uint32_t>(m_frameIndex);
    chunk.weight = m_weight;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

BaseKeyFrame *FaceKeyFrame::clone() const
{
    FaceKeyFrame *frame = new FaceKeyFrame();
    internal::copyBytes(frame->m_name, m_name, kNameSize);
    frame->m_frameIndex = m_frameIndex;
    frame->m_weight = m_weight;
    return frame;
}

const uint8_t *FaceKeyFrame::name() const
{
    return m_name;
}

void FaceKeyFrame::setName(const uint8_t *value)
{
    copyBytesSafe(m_name, value, sizeof(m_name));
}

}
