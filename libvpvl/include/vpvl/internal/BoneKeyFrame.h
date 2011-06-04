/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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

#ifndef VPVL_BONEKEYFRAME_H_
#define VPVL_BONEKEYFRAME_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

class Bone;

class BoneKeyFrame
{
public:
    BoneKeyFrame()
        : m_bone(0),
          m_index(0),
          m_position(0.0f, 0.0f, 0.0f),
          m_rotation(0.0f, 0.0f, 0.0f, 1.0f) {
        memset(m_name, 0, sizeof(m_name));
        memset(m_linear, 0, sizeof(m_linear));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }
    ~BoneKeyFrame() {
        m_bone = 0;
        m_position.setZero();
        m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
        for (int i = 0; i < 4; i++)
            delete m_interpolationTable[i];
        memset(m_name, 0, sizeof(m_name));
        memset(m_linear, 0, sizeof(m_linear));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }

    static const int kTableSize = 64;

    static size_t stride(const char * /* data */) {
        return 15 + sizeof(uint32_t) + sizeof(float) * 7 + 64;
    }

    void read(const char *data) {
        char *ptr = const_cast<char *>(data);
        stringCopySafe(m_name, ptr, sizeof(m_name));
        ptr += sizeof(m_name);
        uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        float pos[3], rot[4];
        internal::vector3(ptr, pos);
        internal::vector4(ptr, rot);
        int8_t table[64];
        memcpy(table, ptr, sizeof(table));
        data += sizeof(table);

        m_index = static_cast<float>(index);
#ifdef VPVL_COORDINATE_OPENGL
        m_position.setValue(pos[0], pos[1], -pos[2]);
        m_rotation.setValue(-rot[0], -rot[1], rot[2], rot[3]);
#else
        m_position.setValue(pos[0], pos[1], pos[2]);
        m_rotation.setValue(rot[0], rot[1], rot[2], rot[3]);
#endif
        setInterpolationTable(table);
    }

    const char *name() const {
        return m_name;
    }
    Bone *bone() const {
        return m_bone;
    }
    float index() const {
        return m_index;
    }
    const btVector3 &position() const {
        return m_position;
    }
    const btQuaternion &rotation() const {
        return m_rotation;
    }
    const bool *linear() const {
        return m_linear;
    }
    const float *const *interpolationTable() const {
        return m_interpolationTable;
    }
    void setBone(Bone *value) {
        m_bone = value;
    }

private:
    void setInterpolationTable(const int8_t *table) {
        for (int i = 0; i < 4; i++)
            m_linear[i] = (table[0 + i] == table[4 + i] == table[8 + i] == table[12 + i]) ? true : false;
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

    char m_name[15];
    Bone *m_bone;
    float m_index;
    btVector3 m_position;
    btQuaternion m_rotation;
    bool m_linear[4];
    float *m_interpolationTable[4];
};

}

#endif
