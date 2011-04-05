/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#ifndef MMDME_PMDRIGIDBODY_H_
#define MMDME_PMDRIGIDBODY_H_

#include "MMDME/Common.h"
#include "MMDME/PMDBone.h"
#include "MMDME/PMDFile.h"

namespace MMDAI {

class PMDRigidBody
{
public:
    PMDRigidBody();
    ~PMDRigidBody();

    bool setup(const PMDFile_RigidBody *rb, PMDBone *bone);
    void joinWorld(btDiscreteDynamicsWorld *btWorld);
    void applyTransformToBone();
    void setKinematic(const bool value);

    inline const char *getName() const {
        return m_name;
    }

    inline btRigidBody *getBody() const {
        return m_body;
    }

private:
    void initialize();
    void clear();

    btCollisionShape *m_shape;
    btRigidBody *m_body;
    btMotionState *m_motionState;
    unsigned short m_groupID;
    unsigned short m_groupMask;
    unsigned char m_type;
    PMDBone *m_bone;
    char *m_name;
    bool m_noBone;
    btTransform m_trans;
    btTransform m_transInv;
    btMotionState *m_kinematicMotionState;
    btDiscreteDynamicsWorld *m_world;

    MMDME_DISABLE_COPY_AND_ASSIGN(PMDRigidBody);
};

} /* namespace */

#endif
