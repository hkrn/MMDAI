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

#include "vpvl2/vmd/CameraKeyframe.h"

namespace vpvl2
{
namespace vmd
{

#pragma pack(push, 1)

struct CameraKeyFrameChunk
{
    int frameIndex;
    float distance;
    float position[3];
    float angle[3];
    int8_t interpolationTable[CameraKeyframe::kTableSize];
    int viewAngle;
    uint8_t noPerspective;
};

#pragma pack(pop)

const QuadWord CameraKeyframe::kDefaultInterpolationParameterValue = QuadWord(20, 20, 107, 107);

static void getValueFromTable(const int8_t *table, int i, QuadWord &v)
{
    static const int8_t zero = 0;
    v.setX(btMax(table[i +  0], zero)); // x1
    v.setY(btMax(table[i +  6], zero)); // y1
    v.setZ(btMax(table[i + 12], zero)); // x2
    v.setW(btMax(table[i + 18], zero)); // y2
}

CameraKeyframe::CameraKeyframe()
    : BaseKeyframe(),
      m_distance(0.0f),
      m_fovy(0.0f),
      m_position(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_noPerspective(false)
{
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(m_rawInterpolationTable, sizeof(m_rawInterpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
}

CameraKeyframe::~CameraKeyframe()
{
    m_distance = 0.0f;
    m_fovy = 0.0f;
    m_position.setZero();
    m_angle.setZero();
    m_noPerspective = false;
    for (int i = 0; i < kMax; i++) {
        delete[] m_interpolationTable[i];
        m_interpolationTable[i] = 0;
    }
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(m_rawInterpolationTable, sizeof(m_rawInterpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
}

size_t CameraKeyframe::strideSize()
{
    return sizeof(CameraKeyFrameChunk);
}

size_t CameraKeyframe::stride() const
{
    return strideSize();
}

void CameraKeyframe::setDefaultInterpolationParameter()
{
    for (int i = 0; i < kMax; i++)
        setInterpolationParameter(static_cast<InterpolationType>(i), kDefaultInterpolationParameterValue);
}

void CameraKeyframe::read(const uint8_t *data)
{
    CameraKeyFrameChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
#ifdef VPVL2_BUILD_IOS
    float pos[3], angle[3];
    memcpy(pos, &chunk.position, sizeof(pos));
    memcpy(angle, &chunk.angle, sizeof(angle));
#else
    float *pos = chunk.position;
    float *angle = chunk.angle;
#endif

    setFrameIndex(static_cast<float>(chunk.frameIndex));
    setFovy(static_cast<float>(chunk.viewAngle));
    setPerspective(chunk.noPerspective == 0);
#ifdef VPVL2_COORDINATE_OPENGL
    setDistance(-chunk.distance);
    setPosition(Vector3(pos[0], pos[1], -pos[2]));
    setAngle(Vector3(-degree(angle[0]), -degree(angle[1]), degree(angle[2])));
#else
    setDistance(chunk.distance);
    setPosition(Vector3(pos[0], pos[1], pos[2]));
    setAngle(Vector3(degree(angle[0]), degree(angle[1]), degree(angle[2])));
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

void CameraKeyframe::write(uint8_t *data) const
{
    CameraKeyFrameChunk chunk;
    chunk.frameIndex = static_cast<int>(m_frameIndex);
    chunk.viewAngle = static_cast<int>(m_fovy);
    chunk.noPerspective = m_noPerspective ? 1 : 0;
    chunk.position[0] = m_position.x();
    chunk.position[1] = m_position.y();
    chunk.angle[2] = radian(m_angle.z());
#ifdef VPVL2_COORDINATE_OPENGL
    chunk.distance = -m_distance;
    chunk.angle[0] = -radian(m_angle.x());
    chunk.angle[1] = -radian(m_angle.y());
    chunk.position[2] = -m_position.z();
#else
    chunk.distance = m_distance;
    chunk.angle[0] = radian(m_angle.x());
    chunk.angle[1] = radian(m_angle.y());
    chunk.position[2] = m_position.z();
#endif
    internal::copyBytes(reinterpret_cast<uint8_t *>(chunk.interpolationTable),
                        reinterpret_cast<const uint8_t *>(m_rawInterpolationTable),
                        sizeof(chunk.interpolationTable));
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

ICameraKeyframe *CameraKeyframe::clone() const
{
    CameraKeyframe *frame = new CameraKeyframe();
    internal::copyBytes(reinterpret_cast<uint8_t *>(frame->m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(m_rawInterpolationTable),
                        sizeof(m_rawInterpolationTable));
    frame->m_frameIndex = m_frameIndex;
    frame->m_distance = m_distance;
    frame->m_fovy = m_fovy;
    frame->m_position = m_position;
    frame->m_angle = m_angle;
    frame->m_noPerspective = m_noPerspective;
    frame->m_parameter = m_parameter;
    frame->setInterpolationTable(m_rawInterpolationTable);
    return frame;
}

void CameraKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
    QuadWord &w = getInterpolationParameterInternal(type);
    value.setValue(w.x(), w.y(), w.z(), w.w());
}

void CameraKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
    setInterpolationParameterInternal(type, value);
    int8_t table[kTableSize];
    internal::zerofill(table, sizeof(table));
    for (int i = 0; i < 4; i++) {
        // x1 => QuadWord#x():0
        // x2 => QuadWord#y():1
        // y1 => QuadWord#z():2
        // y2 => QuadWord#w():3
        int index = i * kMax;
        table[index + kX] = static_cast<int8_t>(m_parameter.x[i]);
        table[index + kY] = static_cast<int8_t>(m_parameter.y[i]);
        table[index + kZ] = static_cast<int8_t>(m_parameter.z[i]);
        table[index + kRotation] = static_cast<int8_t>(m_parameter.rotation[i]);
        table[index + kDistance] = static_cast<int8_t>(m_parameter.distance[i]);
        table[index + kFovy] = static_cast<int8_t>(m_parameter.fovy[i]);
    }
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(table), sizeof(table));
    setInterpolationTable(table);
}

void CameraKeyframe::setInterpolationTable(const int8_t *table)
{
    for (int i = 0; i < kMax; i++)
        m_linear[i] = ((table[4 * i] == table[4 * i + 2]) && (table[4 * i + 1] == table[4 * i + 3])) ? true : false;
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

void CameraKeyframe::setInterpolationParameterInternal(InterpolationType type, const QuadWord &value)
{
    QuadWord &w = getInterpolationParameterInternal(type);
    w.setValue(value.x(), value.y(), value.z(), value.w());
}

QuadWord &CameraKeyframe::getInterpolationParameterInternal(InterpolationType type) const
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
    case kDistance:
        return const_cast<QuadWord &>(m_parameter.distance);
    case kFovy:
        return const_cast<QuadWord &>(m_parameter.fovy);
    default:
        static QuadWord q(0.0f, 0.0f, 0.0f, 0.0f);
        return q;
    }
}

void CameraKeyframe::setDistance(float value)
{
    m_distance = value;
}

void CameraKeyframe::setFovy(float value)
{
    m_fovy = value;
}

void CameraKeyframe::setPosition(const Vector3 &value)
{
    m_position = value;
}

void CameraKeyframe::setAngle(const Vector3 &value)
{
    m_angle = value;
}

void CameraKeyframe::setPerspective(bool value)
{
    m_noPerspective = !value;
}

}
}
