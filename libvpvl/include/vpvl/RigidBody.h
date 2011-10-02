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

#ifndef VPVL_RIGIDBODY_H_
#define VPVL_RIGIDBODY_H_

#include "vpvl/Bone.h"

class btCollisionShape;
class btRigidBody;
class btMotionState;

namespace vpvl
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * RigidBody class represents a rigid body of a Polygon Model Data object.
 */

class VPVL_API RigidBody
{
public:
    RigidBody();
    ~RigidBody();

    static const int kNameSize = 20;

    static size_t stride();

    void read(const uint8_t *data, BoneList *bones);
    void transformBone();
    void setKinematic(bool value);

    const uint8_t *name() const {
        return m_name;
    }
    btRigidBody *body() const {
        return m_body;
    }
    uint16_t groupID() const {
        return m_groupID;
    }
    uint16_t groupMask() const {
        return m_groupMask;
    }

    void setName(const uint8_t *value) {
        copyBytesSafe(m_name, value, sizeof(m_name));
    }

private:
    uint8_t m_name[kNameSize];
    Bone *m_bone;
    btCollisionShape *m_shape;
    btRigidBody *m_body;
    btMotionState *m_motionState;
    Transform m_transform;
    Transform m_invertedTransform;
    btMotionState *m_kinematicMotionState;
    uint16_t m_groupID;
    uint16_t m_groupMask;
    uint8_t m_type;
    bool m_noBone;

    VPVL_DISABLE_COPY_AND_ASSIGN(RigidBody)
};

typedef Array<RigidBody*> RigidBodyList;

} /* namespace vpvl */

#endif
