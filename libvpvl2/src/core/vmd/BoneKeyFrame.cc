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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/vmd/BoneKeyframe.h"

namespace vpvl2
{
namespace vmd
{

#pragma pack(push, 1)

struct BoneKeyFrameChunk
{
    uint8_t name[BoneKeyframe::kNameSize];
    int frameIndex;
    float position[3];
    float rotation[4];
    int8_t interpolationTable[BoneKeyframe::kTableSize];
};

#pragma pack(pop)

const QuadWord BoneKeyframe::kDefaultInterpolationParameterValue = QuadWord(20, 20, 107, 107);

static void getValueFromTable(const int8_t *table, int i, QuadWord &v)
{
    static const int8_t zero = 0;
    v.setX(btMax(table[i +  0], zero)); // x1
    v.setY(btMax(table[i +  4], zero)); // y1
    v.setZ(btMax(table[i +  8], zero)); // x2
    v.setW(btMax(table[i + 12], zero)); // y2
}

size_t BoneKeyframe::strideSize()
{
    return sizeof(BoneKeyFrameChunk);
}

BoneKeyframe::BoneKeyframe(IEncoding *encoding)
    : BaseKeyframe(),
      m_ptr(0),
      m_encoding(encoding),
      m_position(0.0f, 0.0f, 0.0f),
      m_rotation(Quaternion::getIdentity()),
      m_enableIK(true)
{
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(m_rawInterpolationTable, sizeof(m_rawInterpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
}

BoneKeyframe::~BoneKeyframe()
{
    m_encoding = 0;
    m_position.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_enableIK = false;
    delete m_ptr;
    m_ptr = 0;
    for (int i = 0; i < kMax; i++)
        delete[] m_interpolationTable[i];
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(m_rawInterpolationTable, sizeof(m_rawInterpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
}

void BoneKeyframe::setDefaultInterpolationParameter()
{
    for (int i = 0; i < kMax; i++)
        setInterpolationParameter(static_cast<InterpolationType>(i), kDefaultInterpolationParameterValue);
}

void BoneKeyframe::read(const uint8_t *data)
{
    BoneKeyFrameChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
#ifdef VPVL2_BUILD_IOS
    float pos[3], rot[4];
    memcpy(pos, &chunk.position, sizeof(pos));
    memcpy(rot, &chunk.rotation, sizeof(rot));
#else
    float *pos = chunk.position;
    float *rot = chunk.rotation;
#endif
    IString *name = m_encoding->toString(chunk.name, IString::kShiftJIS, sizeof(chunk.name));
    setName(name);
    delete name;
    setFrameIndex(static_cast<float>(chunk.frameIndex));
#ifdef VPVL2_COORDINATE_OPENGL
    setPosition(Vector3(pos[0], pos[1], -pos[2]));
    setRotation(Quaternion(-rot[0], -rot[1], rot[2], rot[3]));
#else
    setPosition(Vector3(pos[0], pos[1], pos[2]));
    setRotation(Quaternion(rot[0], rot[1], rot[2], rot[3]));
#endif
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(chunk.interpolationTable),
                        sizeof(chunk.interpolationTable));
    QuadWord v;
    for (int i = 0; i < kMax; i++) {
        getValueFromTable(m_rawInterpolationTable, i, v);
        setInterpolationParameterInternal(static_cast<InterpolationType>(i), v);
    }
    setInterpolationTable(m_rawInterpolationTable);
}

void BoneKeyframe::write(uint8_t *data) const
{
    BoneKeyFrameChunk chunk;
    uint8_t *name = m_encoding->toByteArray(m_name, IString::kShiftJIS);
    internal::copyBytes(chunk.name, name, sizeof(chunk.name));
    m_encoding->disposeByteArray(name);
    chunk.frameIndex = static_cast<int>(m_frameIndex);
    chunk.position[0] = m_position.x();
    chunk.position[1] = m_position.y();
    chunk.rotation[2] = m_rotation.z();
    chunk.rotation[3] = m_rotation.w();
#ifdef VPVL2_COORDINATE_OPENGL
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

size_t BoneKeyframe::estimateSize() const
{
    return strideSize();
}

IBoneKeyframe *BoneKeyframe::clone() const
{
    BoneKeyframe *frame = m_ptr = new BoneKeyframe(m_encoding);
    frame->setName(m_name);
    internal::copyBytes(reinterpret_cast<uint8_t *>(frame->m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(m_rawInterpolationTable),
                        sizeof(m_rawInterpolationTable));
    frame->setFrameIndex(m_frameIndex);
    frame->setPosition(m_position);
    frame->setRotation(m_rotation);
    frame->m_parameter = m_parameter;
    frame->setInterpolationTable(m_rawInterpolationTable);
    m_ptr = 0;
    return frame;
}

void BoneKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
    QuadWord &w = getInterpolationParameterInternal(type);
    value.setValue(w.x(), w.y(), w.z(), w.w());
}

void BoneKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
    setInterpolationParameterInternal(type, value);
    int8_t table[kTableSize];
    internal::zerofill(table, sizeof(table));
    for (int i = 0; i < 4; i++) {
        // x1 => QuadWord#x():0
        // y1 => QuadWord#y():1
        // x2 => QuadWord#z():2
        // y2 => QuadWord#w():3
        int index = i * kMax;
        table[index + kX] = static_cast<int8_t>(m_parameter.x[i]);
        table[index + kY] = static_cast<int8_t>(m_parameter.y[i]);
        table[index + kZ] = static_cast<int8_t>(m_parameter.z[i]);
        table[index + kRotation] = static_cast<int8_t>(m_parameter.rotation[i]);
    }
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(table), sizeof(table));
    setInterpolationTable(table);
}

void BoneKeyframe::setInterpolationTable(const int8_t *table)
{
    for (int i = 0; i < kMax; i++)
        m_linear[i] = (table[0 + i] == table[4 + i] && table[8 + i] == table[12 + i]) ? true : false;
    QuadWord v;
    for (int i = 0; i < kMax; i++) {
        getValueFromTable(table, i, v);
        delete[] m_interpolationTable[i];
        if (m_linear[i]) {
            m_interpolationTable[i] = 0;
            setInterpolationParameterInternal(static_cast<InterpolationType>(i), v);
            continue;
        }
        m_interpolationTable[i] = new float[kTableSize + 1];
        internal::buildInterpolationTable(v.x() / 127.0f, // x1
                                          v.z() / 127.0f, // x2
                                          v.y() / 127.0f, // y1
                                          v.w() / 127.0f, // y2
                                          kTableSize,
                                          m_interpolationTable[i]);
    }
}

void BoneKeyframe::setInterpolationParameterInternal(InterpolationType type, const QuadWord &value)
{
    QuadWord &w = getInterpolationParameterInternal(type);
    w.setValue(value.x(), value.y(), value.z(), value.w());
}

QuadWord &BoneKeyframe::getInterpolationParameterInternal(InterpolationType type) const
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
    default:
        static QuadWord q(0.0f, 0.0f, 0.0f, 0.0f);
        return q;
    }
}

void BoneKeyframe::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void BoneKeyframe::setPosition(const Vector3 &value)
{
    m_position = value;
}

void BoneKeyframe::setRotation(const Quaternion &value)
{
    m_rotation = value;
}

void BoneKeyframe::setIKEnable(bool value)
{
    m_enableIK = value;
}

}
}
