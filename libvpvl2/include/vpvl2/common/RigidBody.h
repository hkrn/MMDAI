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

#ifndef VPVL2_COMMON_RIGIDBODY_H_
#define VPVL2_COMMON_RIGIDBODY_H_

#include "vpvl2/Common.h"

class btCollisionShape;
class btRigidBody;
class btMotionState;

namespace vpvl2
{

class IBone;
class IString;

namespace common
{

class VPVL2_API RigidBody
{
public:
    enum ShapeType {
        kUnknownShape = -1,
        kSphereShape,
        kBoxShape,
        kCapsureShape,
        kMaxShapeType
    };
    enum ObjectType {
        kStaticObject,
        kDynamicObject,
        kAlignedObject,
        kMaxObjectType
    };

    RigidBody();
    virtual ~RigidBody();

    void performTransformBone();
    void setKinematic(bool value);
    const Transform createStartTransform() const;
    btCollisionShape *createShape() const;
    btRigidBody *createRigidBody(btCollisionShape *shape);

    btRigidBody *body() const { return m_body; }
    IBone *bone() const { return m_boneRef; }
    int boneIndex() const { return m_boneIndex; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const Vector3 &size() const { return m_size; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &rotation() const { return m_rotation; }
    float mass() const { return m_mass; }
    float linearDamping() const { return m_linearDamping; }
    float angularDamping() const { return m_angularDamping; }
    float restitution() const { return m_restitution; }
    float friction() const { return m_friction; }
    uint16_t groupID() const { return m_groupID; }
    uint16_t collisionGroupMask() const { return m_collisionGroupMask; }
    uint8_t collisionGroupID() const { return m_collisionGroupID; }
    int index() const { return m_index; }

    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setBone(IBone *value);
    void setAngularDamping(float value);
    void setCollisionGroupID(uint16_t value);
    void setCollisionMask(uint16_t value);
    void setFriction(float value);
    void setLinearDamping(float value);
    void setMass(float value);
    void setPosition(const Vector3 &value);
    void setRestitution(float value);
    void setRotation(const Vector3 &value);
    void setShapeType(ShapeType value);
    void setSize(const Vector3 &value);
    void setType(ObjectType value);
    void setIndex(int value);

protected:
    void build(IBone *bone, btCollisionShape *shape, int index);

    btRigidBody *m_body;
    btRigidBody *m_ptr;
    btCollisionShape *m_shape;
    btMotionState *m_motionState;
    btMotionState *m_kinematicMotionState;
    Transform m_worldTransform;
    Transform m_world2LocalTransform;
    IBone *m_boneRef;
    IString *m_name;
    IString *m_englishName;
    int m_boneIndex;
    Vector3 m_size;
    Vector3 m_position;
    Vector3 m_rotation;
    float m_mass;
    float m_linearDamping;
    float m_angularDamping;
    float m_restitution;
    float m_friction;
    int m_index;
    uint16_t m_groupID;
    uint16_t m_collisionGroupMask;
    uint8_t m_collisionGroupID;
    ShapeType m_shapeType;
    ObjectType m_type;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RigidBody)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

