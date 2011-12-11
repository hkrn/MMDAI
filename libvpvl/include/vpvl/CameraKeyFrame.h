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

#include "vpvl/BaseKeyFrame.h"

namespace vpvl
{

class VPVL_API CameraKeyFrame : public BaseKeyFrame
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
    struct InterpolationParameter
    {
        QuadWord x;
        QuadWord y;
        QuadWord z;
        QuadWord rotation;
        QuadWord distance;
        QuadWord fovy;
    };

    CameraKeyFrame();
    ~CameraKeyFrame();

    static const int kTableSize = 24;
    static const QuadWord kDefaultInterpolationParameterValue;

    static size_t strideSize();
    size_t stride() const;
    void read(const uint8_t *data);
    void write(uint8_t *data) const;
    BaseKeyFrame *clone() const;

    /**
     * Set the default values of the interpolation parameter.
     */
    void setDefaultInterpolationParameter();

    /**
     * Returns the interpolation values with the type.
     *
     * @param type An interpolation type
     * @param An interpolation parameter to be get
     */
    void getInterpolationParameter(InterpolationType type, QuadWord &value) const;

    /**
     * Set the interpolation values with the type.
     *
     * @param An interpolation type
     * @param An interpolation parameter to set
     */
    void setInterpolationParameter(InterpolationType type, const QuadWord &value);

    /**
     * Returns empty name
     *
     * The camera key frame doesn't have name.
     *
     * @return null name
     */
    const uint8_t *name() const;

    /**
     * Returns the distance of this keyframe.
     *
     * @return A value of distance value
     */
    float distance() const {
        return m_distance;
    }

    /**
     * Returns the fovy of this keyframe.
     *
     * @return A value of fovy value
     */
    float fovy() const {
        return m_fovy;
    }

    /**
     * Returns the position of this keyframe.
     *
     * @return A value of position value
     */
    const Vector3 &position() const {
        return m_position;
    }

    /**
     * Returns the angle of this keyframe.
     *
     * @return A value of angle value
     */
    const Vector3 &angle() const {
        return m_angle;
    }

    /**
     * Returns if this keyframe is perspective.
     *
     * @return True if perspective
     */
    bool isPerspective() const {
        return !m_noPerspective;
    }

    /**
     * Returns whether this keyframe is linear.
     *
     * @return True if this keyframe is linear
     */
    const bool *linear() const {
        return m_linear;
    }

    /**
     * Returns the interpolation values of this keyframe.
     *
     * @return An array of interpolation values
     */
    const float *const *interpolationTable() const {
        return m_interpolationTable;
    }

    /**
     * Set the name of this keyframe.
     *
     * This method do nothing.
     *
     * @param value A value of name
     */
    void setName(const uint8_t *name);

    /**
     * Set the distance of this keyframe.
     *
     * @param value A value of distance
     */
    void setDistance(float value) {
        m_distance = value;
    }

    /**
     * Set the fovy of this keyframe.
     *
     * @param value A value of fovy
     */
    void setFovy(float value) {
        m_fovy = value;
    }

    /**
     * Set the position of this keyframe.
     *
     * @param value A value of position
     */
    void setPosition(const Vector3 &value) {
        m_position = value;
    }

    /**
     * Set the angle of this keyframe.
     *
     * @param value A value of angle
     */
    void setAngle(const Vector3 &value) {
        m_angle = value;
    }

    /**
     * Set boolean this keyframe is perspective.
     *
     * @return True if perspective
     */
    void setPerspective(bool value) {
        m_noPerspective = !value;
    }

private:
    void setInterpolationTable(const int8_t *table);
    void setInterpolationParameterInternal(InterpolationType type, const QuadWord &value);
    QuadWord &getInterpolationParameterInternal(InterpolationType type) const;

    uint8_t m_name[2];
    float m_distance;
    float m_fovy;
    Vector3 m_position;
    Vector3 m_angle;
    bool m_noPerspective;
    bool m_linear[6];
    float *m_interpolationTable[6];
    int8_t m_rawInterpolationTable[kTableSize];
    InterpolationParameter m_parameter;

    VPVL_DISABLE_COPY_AND_ASSIGN(CameraKeyFrame)
};

}

#endif
