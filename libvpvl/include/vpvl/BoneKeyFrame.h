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

#ifndef VPVL_BONEKEYFRAME_H_
#define VPVL_BONEKEYFRAME_H_

#include "vpvl/BaseKeyFrame.h"

namespace vpvl
{

class Bone;

class VPVL_API BoneKeyFrame : public BaseKeyFrame
{
public:
    enum InterpolationType
    {
        kX = 0,
        kY,
        kZ,
        kRotation,
        kMax
    };
    struct InterpolationParameter
    {
        QuadWord x;
        QuadWord y;
        QuadWord z;
        QuadWord rotation;
    };

    BoneKeyFrame();
    ~BoneKeyFrame();

    static const int kNameSize = 15;
    static const int kTableSize = 64;
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
     * Returns the target bone name of this keyframe.
     *
     * @return the bone name
     */
    const uint8_t *name() const;

    /**
     * Returns the position to the target bone of this keyframe.
     *
     * @return A value of position value
     */
    const Vector3 &position() const {
        return m_position;
    }

    /**
     * Returns the rotation to the target bone of this keyframe.
     *
     * @return A value of rotation value
     */
    const Quaternion &rotation() const {
        return m_rotation;
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
     * Set the target bone name of this keyframe.
     *
     * @param value the bone name
     */
    void setName(const uint8_t *value);

    /**
     * Set the position to the target bone of this keyframe.
     *
     * @param value A value of position value
     */
    void setPosition(const Vector3 &value);

    /**
     * Set the rotation to the target bone of this keyframe.
     *
     * @param value A value of rotation value
     */
    void setRotation(const Quaternion &value);

private:
    void setInterpolationTable(const int8_t *table);
    void setInterpolationParameterInternal(InterpolationType type, const QuadWord &value);
    QuadWord &getInterpolationParameterInternal(InterpolationType type) const;

    uint8_t m_name[kNameSize + 1];
    Vector3 m_position;
    Quaternion m_rotation;
    bool m_linear[4];
    float *m_interpolationTable[4];
    int8_t m_rawInterpolationTable[kTableSize];
    InterpolationParameter m_parameter;

    VPVL_DISABLE_COPY_AND_ASSIGN(BoneKeyFrame)
};

}

#endif
