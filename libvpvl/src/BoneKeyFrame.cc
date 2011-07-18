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
    uint8_t name[15];
    uint32_t frameIndex;
    float position[3];
    float rotation[4];
    int8_t interpolationTable[64];
};

#pragma pack(pop)

BoneKeyFrame::BoneKeyFrame()
    : m_frameIndex(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f) {
    internal::zerofill(m_name, sizeof(m_name));
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
}

BoneKeyFrame::~BoneKeyFrame() {
    m_position.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 4; i++)
        delete[] m_interpolationTable[i];
    internal::zerofill(m_name, sizeof(m_name));
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
}

size_t BoneKeyFrame::stride() {
    return sizeof(BoneKeyFrameChunk);
}

void BoneKeyFrame::read(const uint8_t *data) {
    BoneKeyFrameChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    copyBytesSafe(m_name, chunk.name, sizeof(m_name));
    float *pos = chunk.position;
    float *rot = chunk.rotation;

    m_frameIndex = static_cast<float>(chunk.frameIndex);
#ifdef VPVL_COORDINATE_OPENGL
    m_position.setValue(pos[0], pos[1], -pos[2]);
    m_rotation.setValue(-rot[0], -rot[1], rot[2], rot[3]);
#else
    m_position.setValue(pos[0], pos[1], pos[2]);
    m_rotation.setValue(rot[0], rot[1], rot[2], rot[3]);
#endif
    setInterpolationTable(chunk.interpolationTable);
}

void BoneKeyFrame::write(uint8_t *data)
{
    BoneKeyFrameChunk chunk;
    copyBytesSafe(chunk.name, m_name, sizeof(chunk.name));
    chunk.frameIndex = m_frameIndex;
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
                        reinterpret_cast<uint8_t *>(m_interpolationTable),
                        sizeof(chunk.interpolationTable));
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

void BoneKeyFrame::setInterpolationTable(const int8_t *table) {
    for (int i = 0; i < 4; i++)
        m_linear[i] = (table[0 + i] == table[4 + i] && table[8 + i] == table[12 + i]) ? true : false;
    for (int i = 0; i < 4; i++) {
        if (m_linear[i]) {
            m_interpolationTable[i] = 0;
            continue;
        }
        m_interpolationTable[i] = new float[kTableSize + 1];
        float x1 = table[i]      / 127.0f;
        float y1 = table[i +  4] / 127.0f;
        float x2 = table[i +  8] / 127.0f;
        float y2 = table[i + 12] / 127.0f;
        internal::buildInterpolationTable(x1, x2, y1, y2, kTableSize, m_interpolationTable[i]);
    }
}

}
