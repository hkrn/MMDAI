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

#ifndef MMDME_MOTIONMANAGER_H_
#define MMDME_MOTIONMANAGER_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/Common.h"
#include "MMDME/MotionController.h"
#include "MMDME/PMDModel.h"
#include "MMDME/VMD.h"

namespace MMDAI {

enum {
    MOTION_STATUS_RUNNING,
    MOTION_STATUS_LOOPED,
    MOTION_STATUS_DELETED,
};

typedef struct _MotionPlayer {
    char *name;
    MotionController mc;
    VMD *vmd;
    unsigned char onEnd;
    short priority;
    bool ignoreStatic;
    float loopAt;
    bool enableSmooth;
    bool enableRePos;
    float endingBoneBlendFrames;
    float endingFaceBlendFrames;
    float motionBlendRate;
    bool active;
    float endingBoneBlend;
    float endingFaceBlend;
    int statusFlag;
    struct _MotionPlayer *next;
} MotionPlayer;

void MotionPlayer_initialize(MotionPlayer *m);

class MotionManager
{
public:
    static const int kDefaultPriority = 0;
    static const float kDefaultLoopAtFrame;

    MotionManager(PMDModel *pmd);
    ~MotionManager();

    bool startMotion(VMD *vmd, const char *name, bool full, bool once, bool enableSmooth, bool enableRePos);
    void startMotionSub(VMD *vmd, MotionPlayer *m);
    bool swapMotion(VMD *vmd, const char *name);
    bool deleteMotion(const char *name);
    bool update(double frame);
    MotionPlayer *getMotionPlayerList() const;

private:
    void purgeMotion();
    void setup(PMDModel *pmd);
    void initialize();
    void clear();

    PMDModel *m_pmd;
    MotionPlayer *m_playerList;
    float m_beginningNonControlledBlend;

    MMDME_DISABLE_COPY_AND_ASSIGN(MotionManager);
};

} /* namespace */

#endif

