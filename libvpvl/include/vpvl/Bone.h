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

#ifndef VPVL_BONE_H_
#define VPVL_BONE_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

enum BoneType {
    kRotate,
    kRotateAndMove,
    kIKDestination,
    kUnknown,
    kUnderIK,
    kUnderRotate,
    kIKTarget,
    kInvisible,
    kTwist,
    kFollowRotate
};

class Bone;
typedef btAlignedObjectArray<Bone*> BoneList;

class Bone
{
public:
    Bone();
    ~Bone();

    static const uint8_t *centerBoneName();
    static Bone *centerBone(const BoneList *bones);
    static size_t stride(const uint8_t *data);

    void read(const uint8_t *data, BoneList *bones, Bone *rootBone);
    void computeOffset();
    void reset();
    void setMotionIndependency();
    void updateRotation();
    void updateTransform();
    void updateTransform(const btQuaternion &q);
    void getSkinTransform(btTransform &tr);

    const uint8_t *name() const {
        return m_name;
    }
    BoneType type() const {
        return m_type;
    }
    const btTransform &currentTransform() const {
        return m_currentTransform;
    }
    const btVector3 &offset() const {
        return m_offset;
    }
    const btVector3 &originPosition() const {
        return m_originPosition;
    }
    const btVector3 &currentPosition() const {
        return m_currentPosition;
    }
    const btQuaternion &currentRotation() const {
        return m_currentRotation;
    }
    bool isAngleXLimited() const {
        return m_angleXLimited;
    }
    bool isSimulated() const {
        return m_simulated;
    }
    bool hasMotionIndependency() const {
        return m_motionIndepent;
    }

    void setName(const uint8_t *value) {
        copyBytesSafe(m_name, value, sizeof(m_name));
    }
    void setCurrentTransform(const btTransform &value) {
        m_currentTransform = value;
    }
    void setOffset(const btVector3 &value) {
        m_offset = value;
    }
    void setCurrentPosition(const btVector3 &value) {
        m_currentPosition = value;
    }
    void setCurrentRotation(const btQuaternion &value) {
        m_currentRotation = value;
    }
    void setSimulated(bool value) {
        m_simulated = value;
    }

private:
    uint8_t m_name[20];
    BoneType m_type;
    btTransform m_currentTransform;
    btTransform m_transformMoveToOrigin;
    btVector3 m_originPosition;
    btVector3 m_currentPosition;
    btVector3 m_offset;
    btQuaternion m_currentRotation;
    float m_rotateCoef;
    Bone *m_parentBone;
    Bone *m_childBone;
    Bone *m_targetBone;
    bool m_parentIsRoot;
    bool m_angleXLimited;
    bool m_simulated;
    bool m_motionIndepent;
};

} /* namespace vpvl */

#endif
