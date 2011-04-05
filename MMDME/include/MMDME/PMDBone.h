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

    bool setup(const PMDFile_Bone *b, PMDBone *boneList, const uint16_t maxBones, PMDBone *rootBone);
    void computeOffset();
    void reset();
    void setMotionIndependency();
    void updateRotate();
    void update();
    void calcSkinningTrans(btTransform *b);

    inline const char *getName() const {
        return m_name;
    }
    inline unsigned char getType() const {
        return m_type;
    }
    inline const btTransform &getTransform() const {
        return m_trans;
    }
    inline void setTransform(const btTransform &value) {
        m_trans = value;
    }
    inline const btVector3 &getOriginPosition() const {
        return m_originPosition;
    }
    inline bool isLimitAngleX() const {
        return m_limitAngleX;
    }
    inline bool hasMotionIndependency() const {
        return m_motionIndependent;
    }
    inline void setSimulated(bool value) {
        m_simulated = value;
    }
    inline bool isSimulated() const {
        return m_simulated;
    }
    inline const btVector3 &getOffset() const {
        return m_offset;
    }
    inline void setOffset(const btVector3 &value) {
        m_offset = value;
    }
    inline PMDBone *getParentBone() const {
        return m_parentBone;
    }
    inline const btVector3 &getCurrentPosition() const {
        return m_pos;
    }
    inline void setCurrentPosition(const btVector3 &value) {
        m_pos = value;
    }
    inline const btQuaternion &getCurrentRotation() {
        return m_rot;
    }
    inline void setCurrentRotation(const btQuaternion &value) {
        m_rot = value;
    }

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
