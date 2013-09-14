/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_INTERNAL_BASERIGIDBODY_H_
#define VPVL2_INTERNAL_BASERIGIDBODY_H_

#include "vpvl2/IRigidBody.h"
#include "LinearMath/btMotionState.h"

class btCollisionShape;
class btDiscreteDynamicsWorld;
class btRigidBody;

namespace vpvl2
{

class IBone;
class IEncoding;
class IString;

namespace internal
{

class VPVL2_API BaseRigidBody : public IRigidBody
{
public:
    class DefaultMotionState : public btMotionState {
    public:
        DefaultMotionState(const Transform &startTransform, const IBone *boneRef);
        ~DefaultMotionState();

        void getWorldTransform(btTransform &worldTransform) const;
        void setWorldTransform(const btTransform &worldTransform);

        void updateWorldTransform(const btTransform &value);
        const IBone *boneRef() const;
        void setBoneRef(const IBone *value);

    protected:
        const IBone *m_boneRef;
        const Transform m_startTransform;
        Transform m_worldTransform;
    };

    class AlignedMotionState : public DefaultMotionState {
    public:
        AlignedMotionState(const Transform &startTransform, const IBone *boneRef);
        ~AlignedMotionState();

        void getWorldTransform(btTransform &worldTransform) const;
    };

    class KinematicMotionState : public DefaultMotionState {
    public:
        KinematicMotionState(const Transform &startTransform, const IBone *boneRef);
        ~KinematicMotionState();

        void getWorldTransform(btTransform &worldTransform) const;
    };

    BaseRigidBody(IModel *parentModelRef, IEncoding *encodingRef);
    ~BaseRigidBody();

    void addEventListenerRef(PropertyEventListener *value);
    void removeEventListenerRef(PropertyEventListener *value);
    void getEventListenerRefs(Array<PropertyEventListener *> &value);

    void syncLocalTransform();
    void joinWorld(void *value);
    void leaveWorld(void *value);
    void setKinematic(bool value);
    void resetBody(btDiscreteDynamicsWorld *world);

    virtual const Transform createTransform() const;
    virtual btCollisionShape *createShape() const;
    virtual btRigidBody *createRigidBody(btCollisionShape *shape);

    btRigidBody *body() const { return m_body; }
    void *bodyPtr() const { return m_body; }
    IModel *parentModelRef() const { return m_parentModelRef; }
    IBone *boneRef() const { return m_boneRef; }
    int boneIndex() const { return m_boneIndex; }
    const IString *name(IEncoding::LanguageType type) const {
        switch (type) {
        case IEncoding::kDefaultLanguage:
        case IEncoding::kJapanese:
            return m_name;
        case IEncoding::kEnglish:
            return m_englishName;
        default:
            return 0;
        }
    }
    Vector3 size() const { return m_size; }
    Vector3 position() const { return m_position; }
    Vector3 rotation() const { return m_rotation; }
    float32 mass() const { return m_mass; }
    float32 linearDamping() const { return m_linearDamping; }
    float32 angularDamping() const { return m_angularDamping; }
    float32 restitution() const { return m_restitution; }
    float32 friction() const { return m_friction; }
    uint16 groupID() const { return m_groupID; }
    uint16 collisionGroupMask() const { return m_collisionGroupMask; }
    uint8 collisionGroupID() const { return m_collisionGroupID; }
    int index() const { return m_index; }

    void setName(const IString *value, IEncoding::LanguageType type);
    void setParentModelRef(IModel *value);
    void setBoneRef(IBone *value);
    void setAngularDamping(float32 value);
    void setCollisionGroupID(uint8 value);
    void setCollisionMask(uint16 value);
    void setFriction(float32 value);
    void setLinearDamping(float32 value);
    void setMass(float32 value);
    void setPosition(const Vector3 &value);
    void setRestitution(float32 value);
    void setRotation(const Vector3 &value);
    void setShapeType(ShapeType value);
    void setSize(const Vector3 &value);
    void setType(ObjectType value);
    void setIndex(int value);

protected:
    void build(IBone *boneRef, int index);
    virtual DefaultMotionState *createKinematicMotionState() const;
    virtual AlignedMotionState *createAlignedMotionState() const;
    virtual DefaultMotionState *createDefaultMotionState() const;

    btRigidBody *m_body;
    btRigidBody *m_ptr;
    btCollisionShape *m_shape;
    DefaultMotionState *m_motionState;
    DefaultMotionState *m_kinematicMotionState;
    Transform m_worldTransform;
    Transform m_world2LocalTransform;
    IModel *m_parentModelRef;
    IEncoding *m_encodingRef;
    IBone *m_boneRef;
    IString *m_name;
    IString *m_englishName;
    Array<PropertyEventListener *> m_eventRefs;
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
    uint16 m_groupID;
    uint16 m_collisionGroupMask;
    uint8 m_collisionGroupID;
    ShapeType m_shapeType;
    ObjectType m_type;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseRigidBody)
};

} /* namespace internal */
} /* namespace vpvl2 */

#endif

