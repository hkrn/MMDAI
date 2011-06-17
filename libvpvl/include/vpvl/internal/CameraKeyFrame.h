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

#ifndef VPVL_CAMERAKEYFRAME_H_
#define VPVL_CAMERAKEYFRAME_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btVector3.h>
#include "vpvl/common.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

class CameraKeyFrame
{
public:
    CameraKeyFrame()
        : m_index(0),
          m_distance(0.0f),
          m_fovy(0.0f),
          m_position(0.0f, 0.0f, 0.0f),
          m_angle(0.0f, 0.0f, 0.0f),
          m_noPerspective(false)
    {
        memset(m_linear, 0, sizeof(m_linear));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }
    ~CameraKeyFrame() {
        m_index = 0;
        m_distance = 0.0f;
        m_fovy = 0.0f;
        m_position.setZero();
        m_angle.setZero();
        m_noPerspective = false;
        for (int i = 0; i < 6; i++)
            delete[] m_interpolationTable[i];
        memset(m_linear, 0, sizeof(m_linear));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }

    static const int kTableSize = 24;

    static size_t stride() {
        return sizeof(uint32_t) + sizeof(float) * 7 + 24 + sizeof(uint32_t) + sizeof(uint8_t);
    }

    void read(const uint8_t *data) {
        uint8_t *ptr = const_cast<uint8_t *>(data);
        uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        float distance = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
        float pos[3], angle[3];
        internal::vector3(ptr, pos);
        internal::vector3(ptr, angle);
        int8_t table[24];
        memcpy(table, ptr, sizeof(table));
        ptr += sizeof(table);
        uint32_t fovy = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        uint8_t noPerspective = *reinterpret_cast<uint8_t *>(ptr);
        ptr += sizeof(uint8_t);

        m_index = static_cast<float>(index);
        m_fovy = static_cast<float>(fovy);
        m_noPerspective = noPerspective == 1;
    #ifdef VPVL_COORDINATE_OPENGL
        m_distance = -distance;
        m_position.setValue(pos[0], pos[1], -pos[2]);
        m_angle.setValue(-degree(angle[0]), -degree(angle[1]), degree(angle[2]));
    #else
        m_distance = distance;
        m_position.setValue(pos[0], pos[1], pos[2]);
        m_angle.setValue(degree(angle[0]), degree(angle[1]), degree(angle[2]));
    #endif
        setInterpolationTable(table);
    }

    float index() const {
        return m_index;
    }
    float distance() const {
        return m_distance;
    }
    float fovy() const {
        return m_fovy;
    }
    const btVector3 &position() const {
        return m_position;
    }
    const btVector3 &angle() const {
        return m_angle;
    }
    const bool *linear() const {
        return m_linear;
    }
    const float *const *interpolationTable() const {
        return m_interpolationTable;
    }

private:
    void setInterpolationTable(const int8_t *table) {
        for (int i = 0; i < 6; i++)
            m_linear[i] = ((table[4 * i] == table[4 * i + 2]) && (table[4 * i + 1] == table[4 * i + 3])) ? true : false;
        for (int i = 0; i < 6; i++) {
            if (m_linear[i]) {
                m_interpolationTable[i] = 0;
                continue;
            }
            m_interpolationTable[i] = new float[kTableSize + 1];
            float x1 = table[i * 4]     / 127.0f;
            float y1 = table[i * 4 + 2] / 127.0f;
            float x2 = table[i * 4 + 1] / 127.0f;
            float y2 = table[i * 4 + 3] / 127.0f;
            internal::buildInterpolationTable(x1, x2, y1, y2, kTableSize, m_interpolationTable[i]);
        }
    }

    float m_index;
    float m_distance;
    float m_fovy;
    btVector3 m_position;
    btVector3 m_angle;
    bool m_noPerspective;
    bool m_linear[6];
    float *m_interpolationTable[6];
};

}

#endif
