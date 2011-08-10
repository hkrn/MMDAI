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

class VPVL_EXPORT CameraKeyFrame
{
public:
    enum InterpolationType
    {
        kX = 0,
        kY,
        kZ,
        kRotation,
        kDistance,
        kFovy,
        kMax
    };
    typedef struct InterpolationParameter InterpolationParameter;

    CameraKeyFrame();
    ~CameraKeyFrame();

    static const int kTableSize = 24;

    static size_t stride();

    void setDefaultInterpolationParameter();
    void read(const uint8_t *data);
    void write(uint8_t *data);
    void getInterpolationParameter(InterpolationType type, int8_t &x1, int8_t &x2, int8_t &y1, int8_t &y2) const;
    void setInterpolationParameter(InterpolationType type, int8_t x1, int8_t x2, int8_t y1, int8_t y2);

    float frameIndex() const {
        return m_frameIndex;
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

    void setFrameIndex(float value) {
        m_frameIndex = value;
    }
    void setDistance(float value) {
        m_distance = value;
    }
    void setFovy(float value) {
        m_fovy = value;
    }
    void setPosition(const btVector3 &value) {
        m_position = value;
    }
    void setAngle(const btVector3 &value) {
        m_angle = value;
    }

private:
    void setInterpolationTable(const int8_t *table);
    void setInterpolationParameterInternal(InterpolationType type, int8_t x1, int8_t x2, int8_t y1, int8_t y2);
    btQuadWord *getInterpolationParameterInternal(InterpolationType type) const;

    float m_frameIndex;
    float m_distance;
    float m_fovy;
    btVector3 m_position;
    btVector3 m_angle;
    bool m_noPerspective;
    bool m_linear[6];
    float *m_interpolationTable[6];
    int8_t m_rawInterpolationTable[kTableSize];
    InterpolationParameter *m_parameter;

    VPVL_DISABLE_COPY_AND_ASSIGN(CameraKeyFrame)
};

}

#endif
