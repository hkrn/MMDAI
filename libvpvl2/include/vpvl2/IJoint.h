/**

 Copyright (c) 2010-2013  hkrn

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
#ifndef VPVL2_IJOINT_H_
#define VPVL2_IJOINT_H_

#include "vpvl2/IEncoding.h"

namespace vpvl2
{

class IModel;
class IString;
class IRigidBody;

/**
 * モデルの剛体の拘束条件をあらわすインターフェースです。
 *
 */
class VPVL2_API IJoint
{
public:
    enum Type {
        kGeneric6DofSpringConstraint,
        kGeneric6DofConstraint,
        kPoint2PointConstraint,
        kConeTwistConstraint,
        kSliderConstraint,
        kHingeConstraint,
        kMaxType
    };
    class PropertyEventListener {
    public:
        virtual ~PropertyEventListener() {}
        virtual void rigidBody1RefWillChange(IRigidBody *value, IJoint *joint) = 0;
        virtual void rigidBody2RefWillChange(IRigidBody *value, IJoint *joint) = 0;
        virtual void nameWillChange(const IString *value, IEncoding::LanguageType type, IJoint *joint) = 0;
        virtual void positionWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void rotationWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void positionLowerLimitWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void positionUpperLimitWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void rotationLowerLimitWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void rotationUpperLimitWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void positionStiffnessWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void rotationStiffnessWillChange(const Vector3 &value, IJoint *joint) = 0;
        virtual void typeWillChange(IJoint::Type value, IJoint *joint) = 0;
    };

    virtual ~IJoint() {}

    virtual void addEventListenerRef(PropertyEventListener *value) = 0;
    virtual void removeEventListenerRef(PropertyEventListener *value) = 0;
    virtual void getEventListenerRefs(Array<PropertyEventListener *> &value) = 0;

    virtual void *constraintPtr() const = 0;
    virtual IModel *parentModelRef() const = 0;
    virtual IRigidBody *rigidBody1Ref() const = 0;
    virtual IRigidBody *rigidBody2Ref() const = 0;
    virtual int rigidBodyIndex1() const = 0;
    virtual int rigidBodyIndex2() const = 0;
    virtual const IString *name(IEncoding::LanguageType type) const = 0;
    virtual Vector3 position() const = 0;
    virtual Vector3 rotation() const = 0;
    virtual Vector3 positionLowerLimit() const = 0;
    virtual Vector3 positionUpperLimit() const = 0;
    virtual Vector3 rotationLowerLimit() const = 0;
    virtual Vector3 rotationUpperLimit() const = 0;
    virtual Vector3 positionStiffness() const = 0;
    virtual Vector3 rotationStiffness() const = 0;
    virtual Type type() const = 0;
    virtual int index() const = 0;

    virtual void setRigidBody1Ref(IRigidBody *value) = 0;
    virtual void setRigidBody2Ref(IRigidBody *value) = 0;
    virtual void setName(const IString *value, IEncoding::LanguageType type) = 0;
    virtual void setPosition(const Vector3 &value) = 0;
    virtual void setRotation(const Vector3 &value) = 0;
    virtual void setPositionLowerLimit(const Vector3 &value) = 0;
    virtual void setPositionUpperLimit(const Vector3 &value) = 0;
    virtual void setRotationLowerLimit(const Vector3 &value) = 0;
    virtual void setRotationUpperLimit(const Vector3 &value) = 0;
    virtual void setPositionStiffness(const Vector3 &value) = 0;
    virtual void setRotationStiffness(const Vector3 &value) = 0;
    virtual void setType(Type value) = 0;
};

} /* namespace vpvl2 */

#endif
