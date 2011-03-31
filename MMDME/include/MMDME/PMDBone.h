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

#ifndef MMDME_PMDBONE_H_
#define MMDME_PMDBONE_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/Common.h"
#include "MMDME/PMDFile.h"

namespace MMDAI {

#define PMDBONE_KNEENAME "ひざ"
#define PMDBONE_ADDITIONALROOTNAME  "全ての親", "両足オフセ", "右足オフセ", "左足オフセ"
#define PMDBONE_NADDITIONALROOTNAME 4

class PMDBone
{
public:
    PMDBone();
    ~PMDBone();

    bool setup(PMDFile_Bone *b, PMDBone *boneList, unsigned short maxBones, PMDBone *rootBone);
    void computeOffset();
    void reset();
    void setMotionIndependency();
    void updateRotate();
    void update();
    void calcSkinningTrans(btTransform *b);
    const char *getName() const;
    unsigned char getType() const;
    btTransform *getTransform();
    void setTransform(btTransform *tr);
    btVector3 *getOriginPosition();
    bool isLimitAngleX() const;
    bool hasMotionIndependency() const;
    void setSimulatedFlag(bool flag);
    bool isSimulated() const;
    btVector3 *getOffset();
    void setOffset(btVector3 *v);
    PMDBone *getParentBone() const;
    btVector3 *getCurrentPosition();
    void setCurrentPosition(btVector3 *v);
    btQuaternion *getCurrentRotation();
    void setCurrentRotation(btQuaternion *q);

private:
    void initialize();
    void clear();

    char *m_name;
    PMDBone *m_parentBone;
    PMDBone *m_childBone;
    unsigned char m_type;
    PMDBone *m_targetBone;
    btVector3 m_originPosition;
    float m_rotateCoef;
    btVector3 m_offset;
    bool m_parentIsRoot;
    bool m_limitAngleX;
    bool m_motionIndependent;
    btTransform m_trans;
    btTransform m_transMoveToOrigin;
    bool m_simulated;
    btVector3 m_pos;
    btQuaternion m_rot;

    MMDME_DISABLE_COPY_AND_ASSIGN(PMDBone);
};

} /* namespace */

#endif
