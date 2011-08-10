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

#ifndef VPVL_BASEANIMATION_H_
#define VPVL_BASEANIMATION_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * The basic class inherited by BoneAnimation, CameraAnimation and FaceAnimation class.
 */

class VPVL_EXPORT BaseAnimation
{
public:

    /**
     * Constructor that set the smear default value to a given value.
     *
     * @param The smear default value
     */
    BaseAnimation(float smearDefault);

    virtual ~BaseAnimation() {}

    /**
     * Read and parse the buffer with size and sets it's result to the class.
     *
     * @param The buffer to read and parse
     * @param Size of the buffer
     */
    virtual void read(const uint8_t *data, uint32_t size) = 0;

    /**
     * Seek the Animation to the given value index.
     *
     * @param A frame index to seek
     */
    virtual void seek(float frameAt) = 0;

    /**
     * Save the current animation state.
     *
     * @param A position of center
     */
    virtual void takeSnap(const btVector3 &center) = 0;

    /**
     * Seek from the previous to the next frame with delta.
     *
     * If the seeked frame is reached to the max frame of this animation,
     * reached argument set to true.
     *
     * @param A delta frame index to seek the next frame
     * @param A value whether the Animation is reached to the end.
     */
    void advance(float deltaFrame, bool &reached);

    /**
     * Rewind the Animation.
     *
     * @param A frame index to rewind
     * @param A delta frame indx to rewind
     */
    void rewind(float target, float deltaFrame);

    /**
     * Reset all states and last frame index.
     */
    void reset();

    /**
     * Save the current Animation state.
     *
     * @param A position of center
     */
    void setOverrideFirst(const btVector3 &center);

    /**
     * Get the blend rate.
     *
     * @return The blend rate value
     */
    float blendRate() const {
        return m_blendRate;
    }

    /**
     * Get the max frame index.
     *
     * @return The max frame index
     */
    float maxIndex() const {
        return m_maxFrame;
    }

    /**
     * Set the blend rate.
     *
     * @param The blend rate value
     */
    void setBlendRate(float value) {
        m_blendRate = value;
    }

protected:
    uint32_t m_lastIndex;
    uint32_t m_lastLoopStartIndex;
    const float m_smearDefault;
    float m_maxFrame;
    float m_currentFrame;
    float m_previousFrame;
    float m_lastLoopStartFrame;
    float m_blendRate;
    float m_smearIndex;
    bool m_ignoreSingleAnimation;
    bool m_overrideFirst;

    VPVL_DISABLE_COPY_AND_ASSIGN(BaseAnimation)
};

}

#endif

