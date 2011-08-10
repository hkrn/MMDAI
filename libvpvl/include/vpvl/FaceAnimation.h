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

#ifndef VPVL_FACEANIMATION_H_
#define VPVL_FACEANIMATION_H_

#include <LinearMath/btHashMap.h>
#include "vpvl/BaseAnimation.h"

namespace vpvl
{

class Face;
class FaceKeyFrame;
class PMDModel;
typedef struct FaceAnimationInternal FaceAnimationInternal;
typedef btAlignedObjectArray<FaceKeyFrame *> FaceKeyFrameList;

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

class VPVL_EXPORT FaceAnimation : public BaseAnimation
{
public:
    FaceAnimation();
    ~FaceAnimation();

    static const float kStartingMarginFrame;

    void read(const uint8_t *data, uint32_t size);
    void seek(float frameAt);
    void takeSnap(const btVector3 &center);
    void attachModel(PMDModel *model);
    void refresh();
    void reset();

    const FaceKeyFrameList &frames() const {
        return m_frames;
    }
    FaceKeyFrameList *mutableFrames() {
        return &m_frames;
    }
    PMDModel *attachedModel() const {
        return m_model;
    }

private:
    void buildInternalNodes(vpvl::PMDModel *model);
    void calculateFrames(float frameAt, FaceAnimationInternal *node);

    FaceKeyFrameList m_frames;
    btHashMap<btHashString, FaceAnimationInternal *> m_name2node;
    PMDModel *m_model;

    VPVL_DISABLE_COPY_AND_ASSIGN(FaceAnimation)
};

}

#endif
