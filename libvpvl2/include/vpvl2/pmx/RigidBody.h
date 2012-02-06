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

#ifndef VPVL2_PMX_RIGIDBODY_H_
#define VPVL2_PMX_RIGIDBODY_H_

#include "vpvl2/pmx/Model.h"

class btCollisionShape;
class btRigidBody;
class btMotionState;

namespace vpvl2
{
namespace pmx
{

class Bone;
class BoneList;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * RigidBody class represents a rigid body of a Polygon Model Data object.
 */

class VPVL2_API RigidBody
{
public:
    RigidBody();
    ~RigidBody();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadRigidBodies(const Array<RigidBody *> &rigidBodies,
                                const Array<Bone *> &bones);

    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data) const;

    const StaticString *name() const { return m_name; }
    const StaticString *englishName() const { return m_englishName; }
    int boneIndex() const { return m_boneIndex; }
    const Vector3 &size() const { return m_size; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &rotation() const { return m_rotation; }
    float mass() const { return m_mass; }

private:
    btCollisionShape *m_shape;
    btRigidBody *m_body;
    btMotionState *m_motionState;
    Transform m_transform;
    Transform m_invertedTransform;
    btMotionState *m_kinematicMotionState;
    Bone *m_bone;
    StaticString *m_name;
    StaticString *m_englishName;
    int m_boneIndex;
    Vector3 m_size;
    Vector3 m_position;
    Vector3 m_rotation;
    float m_mass;
    uint16_t m_groupID;
    uint16_t m_groupMask;
    uint8_t m_collisionGroupID;
    uint8_t m_shapeType;
    uint8_t m_type;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RigidBody)
};

typedef Array<RigidBody*> RigidBodyList;

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

