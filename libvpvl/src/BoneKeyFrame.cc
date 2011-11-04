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

struct BoneKeyFrameChunk
{
    uint8_t name[BoneKeyFrame::kNameSize];
    int frameIndex;
    float position[3];
    float rotation[4];
    int8_t interpolationTable[BoneKeyFrame::kTableSize];
};

#pragma pack(pop)

static void getValueFromTable(const int8_t *table, int i, int8_t &x1, int8_t &y1, int8_t &x2, int8_t &y2)
{
    static const int8_t zero = 0;
    x1 = btMax(table[i +  0], zero);
    y1 = btMax(table[i +  4], zero);
    x2 = btMax(table[i +  8], zero);
    y2 = btMax(table[i + 12], zero);
}

BoneKeyFrame::BoneKeyFrame()
    : BaseKeyFrame(),
      m_position(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f)
{
    internal::zerofill(m_name, sizeof(m_name));
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(m_rawInterpolationTable, sizeof(m_rawInterpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
}

BoneKeyFrame::~BoneKeyFrame()
{
    m_position.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < kMax; i++)
        delete[] m_interpolationTable[i];
    internal::zerofill(m_name, sizeof(m_name));
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(m_rawInterpolationTable, sizeof(m_rawInterpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
}

size_t BoneKeyFrame::strideSize()
{
    return sizeof(BoneKeyFrameChunk);
}

size_t BoneKeyFrame::stride() const
{
    return strideSize();
}

void BoneKeyFrame::setDefaultInterpolationParameter()
{
    for (int i = 0; i < kMax; i++)
        setInterpolationParameter(static_cast<InterpolationType>(i), 20, 107, 20, 107);
}

void BoneKeyFrame::read(const uint8_t *data)
{
    BoneKeyFrameChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    setName(chunk.name);
#ifdef VPVL_BUILD_IOS
    float pos[3], rot[4];
    memcpy(pos, &chunk.position, sizeof(pos));
    memcpy(rot, &chunk.rotation, sizeof(rot));
#else
    float *pos = chunk.position;
    float *rot = chunk.rotation;
#endif

    m_frameIndex = static_cast<float>(chunk.frameIndex);
#ifdef VPVL_COORDINATE_OPENGL
    m_position.setValue(pos[0], pos[1], -pos[2]);
    m_rotation.setValue(-rot[0], -rot[1], rot[2], rot[3]);
#else
    m_position.setValue(pos[0], pos[1], pos[2]);
    m_rotation.setValue(rot[0], rot[1], rot[2], rot[3]);
#endif
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(chunk.interpolationTable),
                        sizeof(chunk.interpolationTable));
    int8_t x1, y1, x2, y2;
    for (int i = 0; i < kMax; i++) {
        getValueFromTable(m_rawInterpolationTable, i, x1, y1, x2, y2);
        setInterpolationParameterInternal(static_cast<InterpolationType>(i), x1, x2, y1, y2);
    }
    setInterpolationTable(m_rawInterpolationTable);
}

void BoneKeyFrame::write(uint8_t *data) const
{
    BoneKeyFrameChunk chunk;
    internal::copyBytes(chunk.name, m_name, sizeof(chunk.name));
    chunk.frameIndex = static_cast<int>(m_frameIndex);
    chunk.position[0] = m_position.x();
    chunk.position[1] = m_position.y();
    chunk.rotation[2] = m_rotation.z();
    chunk.rotation[3] = m_rotation.w();
#ifdef VPVL_COORDINATE_OPENGL
    chunk.rotation[0] = -m_rotation.x();
    chunk.rotation[1] = -m_rotation.y();
    chunk.position[2] = -m_position.z();
#else
    chunk.rotation[0] = m_rotation.x();
    chunk.rotation[1] = m_rotation.y();
    chunk.position[2] = m_position.z();
#endif
    internal::copyBytes(reinterpret_cast<uint8_t *>(chunk.interpolationTable),
                        reinterpret_cast<const uint8_t *>(m_rawInterpolationTable),
                        sizeof(chunk.interpolationTable));
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

BaseKeyFrame *BoneKeyFrame::clone() const
{
    BoneKeyFrame *frame = new BoneKeyFrame();
    internal::copyBytes(frame->m_name, m_name, kNameSize);
    internal::copyBytes(reinterpret_cast<uint8_t *>(frame->m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(m_rawInterpolationTable),
                        sizeof(m_rawInterpolationTable));
    frame->m_frameIndex = m_frameIndex;
    frame->m_position = m_position;
    frame->m_rotation = m_rotation;
    frame->m_parameter = m_parameter;
    frame->setInterpolationTable(m_rawInterpolationTable);
    return frame;
}

void BoneKeyFrame::getInterpolationParameter(InterpolationType type, int8_t &x1, int8_t &x2, int8_t &y1, int8_t &y2) const
{
    QuadWord &w = getInterpolationParameterInternal(type);
    x1 = static_cast<int8_t>(w.x());
    y1 = static_cast<int8_t>(w.y());
    x2 = static_cast<int8_t>(w.z());
    y2 = static_cast<int8_t>(w.w());
}

void BoneKeyFrame::setInterpolationParameter(InterpolationType type, int8_t x1, int8_t x2, int8_t y1, int8_t y2)
{
    setInterpolationParameterInternal(type, x1, x2, y1, y2);
    int8_t table[kTableSize];
    internal::zerofill(table, sizeof(table));
    for (int i = 0; i < 4; i++) {
        // x1 => QuadWord#x():0
        // y1 => QuadWord#y():1
        // x2 => QuadWord#z():2
        // y2 => QuadWord#w():3
        table[i * kMax + kX] = m_parameter.x[i];
        table[i * kMax + kY] = m_parameter.y[i];
        table[i * kMax + kZ] = m_parameter.z[i];
        table[i * kMax + kRotation] = m_parameter.rotation[i];
    }
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(table), sizeof(table));
    setInterpolationTable(table);
}

const uint8_t *BoneKeyFrame::name() const
{
    return m_name;
}

void BoneKeyFrame::setName(const uint8_t *value)
{
    copyBytesSafe(m_name, value, sizeof(m_name));
}

void BoneKeyFrame::setInterpolationTable(const int8_t *table)
{
    for (int i = 0; i < kMax; i++)
        m_linear[i] = (table[0 + i] == table[4 + i] && table[8 + i] == table[12 + i]) ? true : false;
    int8_t x1, y1, x2, y2;
    for (int i = 0; i < kMax; i++) {
        getValueFromTable(table, i, x1, y1, x2, y2);
        if (m_linear[i]) {
            m_interpolationTable[i] = 0;
            setInterpolationParameterInternal(static_cast<InterpolationType>(i), x1, x2, y1, y2);
            continue;
        }
        delete[] m_interpolationTable[i];
        m_interpolationTable[i] = new float[kTableSize + 1];
        internal::buildInterpolationTable(x1 / 127.0f, x2 / 127.0f, y1 / 127.0f, y2 / 127.0f, kTableSize, m_interpolationTable[i]);
    }
}

void BoneKeyFrame::setInterpolationParameterInternal(InterpolationType type, int8_t x1, int8_t x2, int8_t y1, int8_t y2)
{
    QuadWord &w = getInterpolationParameterInternal(type);
    w.setX(x1);
    w.setY(y1);
    w.setZ(x2);
    w.setW(y2);
}

QuadWord &BoneKeyFrame::getInterpolationParameterInternal(InterpolationType type) const
{
    switch (type) {
    case kX:
        return const_cast<QuadWord &>(m_parameter.x);
    case kY:
        return const_cast<QuadWord &>(m_parameter.y);
    case kZ:
        return const_cast<QuadWord &>(m_parameter.z);
    case kRotation:
        return const_cast<QuadWord &>(m_parameter.rotation);
    }
}

}
