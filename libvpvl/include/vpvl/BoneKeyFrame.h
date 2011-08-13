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

#ifndef VPVL_BONEKEYFRAME_H_
#define VPVL_BONEKEYFRAME_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btVector3.h>
#include "vpvl/BaseKeyFrame.h"

namespace vpvl
{

class Bone;

class VPVL_EXPORT BoneKeyFrame : public BaseKeyFrame
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
        btQuadWord x;
        btQuadWord y;
        btQuadWord z;
        btQuadWord rotation;
    };

    BoneKeyFrame();
    ~BoneKeyFrame();

    static const int kNameSize = 15;
    static const int kTableSize = 64;

    static size_t strideSize();

    size_t stride() const;
    void read(const uint8_t *data);
    void write(uint8_t *data) const;

    /**
     * Set the default values of the interpolation parameter.
     */
    void setDefaultInterpolationParameter();

    /**
     * Get the interpolation values with the type.
     *
     * @param An interpolation type
     * @param A value of X1
     * @param A value of X2
     * @param A value of Y1
     * @param A value of Y2
     */
    void getInterpolationParameter(InterpolationType type, int8_t &x1, int8_t &x2, int8_t &y1, int8_t &y2) const;

    /**
     * Set the interpolation values with the type.
     *
     * @param An interpolation type
     * @param A value of X1
     * @param A value of X2
     * @param A value of Y1
     * @param A value of Y2
     */
    void setInterpolationParameter(InterpolationType type, int8_t x1, int8_t x2, int8_t y1, int8_t y2);

    /**
     * Get the target bone name of this keyframe.
     *
     * @return the bone name
     */
    const uint8_t *name() const;

    /**
     * Get the position to the target bone of this keyframe.
     *
     * @return A value of position value
     */
    const btVector3 &position() const {
        return m_position;
    }

    /**
     * Get the rotation to the target bone of this keyframe.
     *
     * @return A value of rotation value
     */
    const btQuaternion &rotation() const {
        return m_rotation;
    }

    /**
     * Get whether this keyframe is linear.
     *
     * @return True if this keyframe is linear
     */
    const bool *linear() const {
        return m_linear;
    }

    /**
     * Get the interpolation values of this keyframe.
     *
     * @return An array of interpolation values
     */
    const float *const *interpolationTable() const {
        return m_interpolationTable;
    }

    /**
     * Set the target bone name of this keyframe.
     *
     * @param the bone name
     */
    void setName(const uint8_t *value);

    /**
     * Set the position to the target bone of this keyframe.
     *
     * @param A value of position value
     */
    void setPosition(const btVector3 &value) {
        m_position = value;
    }

    /**
     * Set the rotation to the target bone of this keyframe.
     *
     * @param A value of rotation value
     */
    void setRotation(const btQuaternion &value) {
        m_rotation = value;
    }

private:
    void setInterpolationTable(const int8_t *table);
    void setInterpolationParameterInternal(InterpolationType type, int8_t x1, int8_t x2, int8_t y1, int8_t y2);
    btQuadWord &getInterpolationParameterInternal(InterpolationType type) const;

    uint8_t m_name[kNameSize];
    btVector3 m_position;
    btQuaternion m_rotation;
    bool m_linear[4];
    float *m_interpolationTable[4];
    int8_t m_rawInterpolationTable[kTableSize];
    InterpolationParameter m_parameter;

    VPVL_DISABLE_COPY_AND_ASSIGN(BoneKeyFrame)
};

}

#endif
