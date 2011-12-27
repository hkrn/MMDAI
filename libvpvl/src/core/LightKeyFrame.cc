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

struct LightKeyFrameChunk
{
    int frameIndex;
    float color[3];
    float direction[3];
};

#pragma pack(pop)

LightKeyFrame::LightKeyFrame()
    : BaseKeyFrame(),
      m_color(0.0f, 0.0f, 0.0f),
      m_direction(0.0f, 0.0f, 0.0f)
{
    internal::zerofill(m_name, sizeof(m_name));
}
LightKeyFrame::~LightKeyFrame()
{
    internal::zerofill(m_name, sizeof(m_name));
    m_color.setZero();
    m_direction.setZero();
}

size_t LightKeyFrame::strideSize()
{
    return sizeof(LightKeyFrameChunk);
}

size_t LightKeyFrame::stride() const
{
    return strideSize();
}

void LightKeyFrame::read(const uint8_t *data)
{
    LightKeyFrameChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    m_frameIndex = static_cast<float>(chunk.frameIndex);
#ifdef VPVL_BUILD_IOS
    float color[3], direction[4];
    memcpy(color, &chunk.color, sizeof(color));
    memcpy(direction, &chunk.direction, sizeof(direction));
#else
    float *color = chunk.color;
    float *direction = chunk.direction;
#endif
    setColor(Vector3(color[0], color[1], color[2]));
#ifdef VPVL_COORDINATE_OPENGL
    setDirection(Vector3(-direction[0], -direction[1], direction[2]));
#else
    setDirection(Vector3(direction[0], direction[1], direction[2]));
#endif
}

void LightKeyFrame::write(uint8_t *data) const
{
    LightKeyFrameChunk chunk;
    chunk.frameIndex = static_cast<int>(m_frameIndex);
    chunk.color[0] = m_color.x();
    chunk.color[1] = m_color.y();
    chunk.color[2] = m_color.z();
#ifdef VPVL_COORDINATE_OPENGL
    chunk.direction[0] = -m_direction.x();
    chunk.direction[1] = -m_direction.y();
#else
    chunk.direction[0] = m_direction.x();
    chunk.direction[1] = m_direction.y();
#endif
    chunk.direction[2] = m_direction.z();
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

BaseKeyFrame *LightKeyFrame::clone() const
{
    LightKeyFrame *frame = new LightKeyFrame();
    frame->setFrameIndex(m_frameIndex);
    frame->setColor(m_color);
    frame->setDirection(m_direction);
    return frame;
}

const uint8_t *LightKeyFrame::name() const
{
    return m_name;
}

void LightKeyFrame::setName(const uint8_t * /* value */)
{
}

void LightKeyFrame::setColor(const Vector3 &value)
{
    m_color = value;
}

void LightKeyFrame::setDirection(const Vector3 &value)
{
    m_direction = value;
}

}
