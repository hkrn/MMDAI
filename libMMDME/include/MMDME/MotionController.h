/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

#include "MMDME/PMDBone.h"
#include "MMDME/PMDFace.h"
#include "MMDME/PMDModel.h"
#include "MMDME/VMD.h"

#define MOTIONCONTROLLER_BONESTARTMARGINFRAME 20.0f /* frame lengths for bone motion smoothing at loop head */
#define MOTIONCONTROLLER_FACESTARTMARGINFRAME 6.0f  /* frame lengths for face motion smoothing at loop head */

#define MOTIONCONTROLLER_CENTERBONENAME { 0x83, 0x5a, 0x83, 0x93, 0x83, 0x5e, 0x81, 0x5b }

/* MotionControllerBoneElement: motion control element for bone */
typedef struct _MotionControllerBoneElement {
   PMDBone *bone;         /* bone to be controlled */
   BoneMotion *motion;    /* bone motion to be played */
   btVector3 pos;         /* calculated position */
   btQuaternion rot;      /* calculated rotation */
   unsigned long lastKey; /* last key frame number */
   btVector3 snapPos;     /* snapped position, to be used as initial position at frame 0 */
   btQuaternion snapRot;  /* snapped rotation, to be used as initial rotation at frame 0 */
} MotionControllerBoneElement;

/* MotionControllerFaceElement: Motion control element for face */
typedef struct _MotionControllerFaceElement {
   PMDFace *face;         /* face to be managed */
   FaceMotion *motion;    /* face motion to be played */
   float weight;          /* calculated weight */
   float snapWeight;      /* snapped face weight, to be used as initial weight at frame 0 */
   unsigned long lastKey; /* last key frame number */
} MotionControllerFaceElement;

/* MotionController: motion controller class, to handle one motion to a list of bones and faces */
class MotionController
{
private:

   float m_maxFrame; /* maximum key frame */

   unsigned long m_numBoneCtrl;                 /* number of bone control list */
   MotionControllerBoneElement *m_boneCtrlList; /* bone control list */
   unsigned long m_numFaceCtrl;                 /* number of face control list */
   MotionControllerFaceElement *m_faceCtrlList; /* face control list */

   /* values determined by the given PMD and VMD */
   bool m_hasCenterBoneMotion; /* true if the motion has more than 1 key frames for center bone and need re-location */

   /* configurations */
   float m_boneBlendRate;     /* if != 1.0f, the resulting bone location will be blended upon the current bone position at this rate, else override it */
   float m_faceBlendRate;     /* if != 1.0f, the resulting face weight will be blended upon the current face at this rate, else override it */
   bool m_ignoreSingleMotion; /* if true, motions with only one key frame for the first frame will be disgarded. This is for inserting motions */

   /* internal work area */
   double m_currentFrame;      /* current frame */
   double m_previousFrame;     /* current frame at last call to advance() */
   float m_lastLoopStartFrame; /* m_firstFrame frame where the last motion loop starts, used for motion smoothing at loop head */
   float m_noBoneSmearFrame;   /* remaining frames for bone motion smoothing at loop head */
   float m_noFaceSmearFrame;   /* remaining frames for face motion smoothing at loop head */

   /* internal work area for initial pose snapshot */
   bool m_overrideFirst; /* when true, the initial bone pos/rot and face weights in the motion at the first frame will be replaced by the runtime pose snapshot */

   /* calcBoneAt: calculate bone pos/rot at the given frame */
   void calcBoneAt(MotionControllerBoneElement *mc, float absFrame);

   /* calcFaceAt: calculate face weight at the given frame */
   void calcFaceAt(MotionControllerFaceElement *mc, float absFrame);

   /* control: set bone position/rotation and face weights according to the motion to the specified frame */
   void control(float frameNow);

   /* takeSnap: take a snap shot of current model pose for smooth motion insertion / loop */
   void takeSnap(btVector3 *centerPos);

   /* initialize: initialize controller */
   void initialize();

   /* clear: free controller */
   void clear();

public:

   /* MotionController: constructor */
   MotionController();

   /* ~MotionController: destructor */
   ~MotionController();

   /* setup: initialize and set up controller */
   void setup(PMDModel *model, VMD *motions);

   /* reset: reset values */
   void reset();

   /* advance: advance motion controller by the given frame, return true when reached end */
   bool advance(double deltaFrame);

   /* rewind: rewind motion controller to the given frame */
   void rewind(float targetFrame, float frame);

   /* setOverrideFirst: should be called at the first frame, to tell controller to take snapshot */
   void setOverrideFirst(btVector3 *centerPos);

   /* setBoneBlendRate: set bone blend rate */
   void setBoneBlendRate(float rate);

   /* setFaceBlendRate: set face blend rate */
   void setFaceBlendRate(float rate);

   /* setIgnoreSingleMotion: set insert motion flag */
   void setIgnoreSingleMotion(bool val);

   /* hasCenter: return true if the motion has more than 1 key frames for center bone */
   bool hasCenter();

   /* getMaxFrame: get max frame */
   float getMaxFrame();

   /* getCurrentFrame: get current frame */
   double getCurrentFrame();

   /* setCurrentFrame: set current frame */
   void setCurrentFrame(double frame);
};

#endif
