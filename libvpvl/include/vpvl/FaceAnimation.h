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

#ifndef VPVL_FACEANIMATION_H_
#define VPVL_FACEANIMATION_H_

#include "vpvl/BaseAnimation.h"

namespace vpvl
{

class Face;
class FaceKeyframe;
class PMDModel;
typedef Array<FaceKeyframe *> FaceKeyFrameList;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * FaceAnimation class represents a face Animation that includes many face frames
 * of a Vocaloid Motion Data object inherits BaseAnimation.
 */

class VPVL_API FaceAnimation : public BaseAnimation
{
public:
    typedef struct InternalKeyFrameList InternalKeyFrameList;

    FaceAnimation();
    ~FaceAnimation();

    void read(const uint8_t *data, int size);
    void seek(float frameAt);

    /**
     * Attach this to the model.
     *
     * After calling this method, internal states to animate are built.
     * If you modified frames of this animation, you should call
     * refresh method to rebuild internal states.
     *
     * This method has no effect if you have already called it.
     *
     * @param model A model to attach the motion
     * @see refresh
     */
    void attachModel(PMDModel *model);

    /**
     * Rebuild internal states to animate.
     *
     * This method has no effect if you haven't call attachModel.
     *
     * @see attachModel
     */
    void refresh();

    /**
     * Reset the last frame index of all frames.
     *
     */
    void reset();

    /**
     * Get a camera key frame associated with index.
     *
     * @param i A frame index to get key frame
     * @return A camera key frame associated with index
     */
    FaceKeyframe *frameAt(int i) const {
        return static_cast<FaceKeyframe *>(m_frames[i]);
    }

    /**
     * Get an attached model of this animation.
     *
     * @return An attached model
     */
    PMDModel *attachedModel() const {
        return m_model;
    }

    bool isNullFrameEnabled() const {
        return m_enableNullFrame;
    }

    void setNullFrameEnable(bool value) {
        m_enableNullFrame = value;
    }

private:
    void buildInternalNodes(vpvl::PMDModel *model);
    void calculateFrames(float frameAt, InternalKeyFrameList *keyFrames);

    Hash<HashString, InternalKeyFrameList *> m_name2keyframes;
    PMDModel *m_model;
    bool m_enableNullFrame;

    VPVL_DISABLE_COPY_AND_ASSIGN(FaceAnimation)
};

}

#endif
