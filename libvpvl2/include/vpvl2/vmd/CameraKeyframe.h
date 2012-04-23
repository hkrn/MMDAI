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

#ifndef VPVL2_VMD_CAMERAKEYFRAME_H_
#define VPVL2_VMD_CAMERAKEYFRAME_H_

#include "vpvl2/ICameraKeyframe.h"
#include "vpvl2/vmd/BaseKeyframe.h"

namespace vpvl2
{
namespace vmd
{

class VPVL2_API CameraKeyframe : public BaseKeyframe, public ICameraKeyframe
{
public:
    struct InterpolationParameter
    {
        QuadWord x;
        QuadWord y;
        QuadWord z;
        QuadWord rotation;
        QuadWord distance;
        QuadWord fovy;
    };
    static size_t strideSize();

    CameraKeyframe();
    ~CameraKeyframe();

    static const int kTableSize = 24;
    static const QuadWord kDefaultInterpolationParameterValue;

    void read(const uint8_t *data);
    void write(uint8_t *data) const;
    size_t estimateSize() const;
    ICameraKeyframe *clone() const;

    void setName(const IString * /* value */) {}
    void setDefaultInterpolationParameter();
    void getInterpolationParameter(InterpolationType type, QuadWord &value) const;
    void setInterpolationParameter(InterpolationType type, const QuadWord &value);

    float distance() const { return m_distance; }
    float fovy() const { return m_fovy; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &angle() const { return m_angle; }
    bool isPerspective() const { return !m_noPerspective; }
    const bool *linear() const { return m_linear; }
    const float *const *interpolationTable() const { return m_interpolationTable; }
    Type type() const { return IKeyframe::kCamera; }

    void setDistance(float value);
    void setFovy(float value);
    void setPosition(const Vector3 &value);
    void setAngle(const Vector3 &value);
    void setPerspective(bool value);

private:
    void setInterpolationTable(const int8_t *table);
    void setInterpolationParameterInternal(InterpolationType type, const QuadWord &value);
    QuadWord &getInterpolationParameterInternal(InterpolationType type) const;

    mutable CameraKeyframe *m_ptr;
    float m_distance;
    float m_fovy;
    Vector3 m_position;
    Vector3 m_angle;
    bool m_noPerspective;
    bool m_linear[6];
    float *m_interpolationTable[6];
    int8_t m_rawInterpolationTable[kTableSize];
    InterpolationParameter m_parameter;

    VPVL2_DISABLE_COPY_AND_ASSIGN(CameraKeyframe)
};

}
}

#endif
