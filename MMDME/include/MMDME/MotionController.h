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

#ifndef MMDME_MOTIONCONTROLLER_H_
#define MMDME_MOTIONCONTROLLER_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/Common.h"
#include "MMDME/PMDBone.h"
#include "MMDME/PMDFace.h"
#include "MMDME/PMDModel.h"
#include "MMDME/VMD.h"

namespace MMDAI {

const char kCenterBoneName[] = { 0x83, 0x5a, 0x83, 0x93, 0x83, 0x5e, 0x81, 0x5b };

typedef struct _MotionControllerBoneElement {
    PMDBone *bone;
    BoneMotion *motion;
    btVector3 pos;
    btQuaternion rot;
    unsigned long lastKey;
    btVector3 snapPos;
    btQuaternion snapRot;
} MotionControllerBoneElement;

typedef struct _MotionControllerFaceElement {
    PMDFace *face;
    FaceMotion *motion;
    float weight;
    float snapWeight;
    unsigned long lastKey;
} MotionControllerFaceElement;

class MotionController
{
public:
    static const float kBoneStartMarginFrame;
    static const float kFaceStartMarginFrame;

    MotionController();
    ~MotionController();

    void setup(PMDModel *model, VMD *motions);
    void reset();
    bool advance(double deltaFrame);
    void rewind(float targetFrame, float frame);
    void setOverrideFirst(btVector3 *centerPos);
    void setBoneBlendRate(float rate);
    void setFaceBlendRate(float rate);
    void setIgnoreSingleMotion(bool val);
    bool hasCenter() const;
    float getMaxFrame() const;
    double getCurrentFrame() const;
    void setCurrentFrame(double frame);

private:
    void calcBoneAt(MotionControllerBoneElement *mc, float absFrame);
    void calcFaceAt(MotionControllerFaceElement *mc, float absFrame);
    void control(float frameNow);
    void takeSnap(btVector3 *centerPos);
    void initialize();
    void clear();

    float m_maxFrame;
    unsigned long m_numBoneCtrl;
    MotionControllerBoneElement *m_boneCtrlList;
    unsigned long m_numFaceCtrl;
    MotionControllerFaceElement *m_faceCtrlList;
    bool m_hasCenterBoneMotion;
    float m_boneBlendRate;
    float m_faceBlendRate;
    bool m_ignoreSingleMotion;
    double m_currentFrame;
    double m_previousFrame;
    float m_lastLoopStartFrame;
    float m_noBoneSmearFrame;
    float m_noFaceSmearFrame;
    bool m_overrideFirst;

    MMDME_DISABLE_COPY_AND_ASSIGN(MotionController);
};

} /* namespace */

#endif

