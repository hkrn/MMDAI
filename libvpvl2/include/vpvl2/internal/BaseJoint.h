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
#ifndef VPVL2_INTERNAL_BASEJOINT_H_
#define VPVL2_INTERNAL_BASEJOINT_H_

#include "vpvl2/IJoint.h"

#ifndef VPVL2_NO_BULLET
class btTypedConstraint;
class btGeneric6DofSpringConstraint;
#else
BT_DECLARE_HANDLE(btTypedConstraint);
BT_DECLARE_HANDLE(btGeneric6DofSpringConstraint);
#endif

namespace vpvl2
{
namespace internal
{

class VPVL2_API BaseJoint : public IJoint
{
public:
    static const Scalar kDefaultDamping;

    BaseJoint(IModel *modelRef);
    ~BaseJoint();

    void addEventListenerRef(PropertyEventListener *value);
    void removeEventListenerRef(PropertyEventListener *value);
    void getEventListenerRefs(Array<PropertyEventListener *> &value);

    void joinWorld(btDiscreteDynamicsWorld *worldRef);
    void leaveWorld(btDiscreteDynamicsWorld *worldRef);
    void updateTransform();

    btTypedConstraint *constraint() const { return m_constraint; }
    void *constraintPtr() const { return m_constraint; }

    IModel *parentModelRef() const { return m_parentModelRef; }
    IRigidBody *rigidBody1Ref() const { return m_rigidBody1Ref; }
    IRigidBody *rigidBody2Ref() const { return m_rigidBody2Ref; }
    int rigidBodyIndex1() const { return m_rigidBodyIndex1; }
    int rigidBodyIndex2() const { return m_rigidBodyIndex2; }
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
    Vector3 position() const { return m_position; }
    Vector3 rotation() const { return m_rotation; }
    Vector3 positionLowerLimit() const { return m_positionLowerLimit; }
    Vector3 positionUpperLimit() const { return m_positionUpperLimit; }
    Vector3 rotationLowerLimit() const { return m_rotationLowerLimit; }
    Vector3 rotationUpperLimit() const { return m_rotationUpperLimit; }
    Vector3 positionStiffness() const { return m_positionStiffness; }
    Vector3 rotationStiffness() const { return m_rotationStiffness; }
    int index() const { return m_index; }

    void setParentModelRef(IModel *value);
    void setRigidBody1Ref(IRigidBody *value);
    void setRigidBody2Ref(IRigidBody *value);
    void setName(const IString *value, IEncoding::LanguageType type);
    void setPosition(const Vector3 &value);
    void setRotation(const Vector3 &value);
    void setPositionLowerLimit(const Vector3 &value);
    void setPositionUpperLimit(const Vector3 &value);
    void setRotationLowerLimit(const Vector3 &value);
    void setRotationUpperLimit(const Vector3 &value);
    void setPositionStiffness(const Vector3 &value);
    void setRotationStiffness(const Vector3 &value);
    void setIndex(int value);

    virtual btTypedConstraint *createConstraint();

protected:
    btGeneric6DofSpringConstraint *createGeneric6DofSpringConstraint();
    void getJointWorldTransform(Transform &worldTransform) const;
    void build(int index);

    btTypedConstraint *m_constraint;
    btTypedConstraint *m_ptr;
    IModel *m_parentModelRef;
    IRigidBody *m_rigidBody1Ref;
    IRigidBody *m_rigidBody2Ref;
    IString *m_name;
    IString *m_englishName;
    Array<PropertyEventListener *> m_eventRefs;
    Vector3 m_position;
    Vector3 m_rotation;
    Vector3 m_positionLowerLimit;
    Vector3 m_rotationLowerLimit;
    Vector3 m_positionUpperLimit;
    Vector3 m_rotationUpperLimit;
    Vector3 m_positionStiffness;
    Vector3 m_rotationStiffness;
    Type m_type;
    int m_rigidBodyIndex1;
    int m_rigidBodyIndex2;
    int m_index;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseJoint)
};

} /* namespace internal */
} /* namespace vpvl2 */

#endif
