/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#pragma once
#ifndef VPVL2_QT_WORLD_H_
#define VPVL2_QT_WORLD_H_

#include "vpvl2/qt/Common.h"
#include "vpvl2/Scene.h"

#include <QScopedPointer>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>

class btCollisionDispatcher;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btSequentialImpulseConstraintSolver;
struct btDbvtBroadphase;

namespace vpvl2
{
namespace qt
{

class VPVL2QTCOMMON_API World
{
public:
    static const Vector3 kAabbSize;
    static const Vector3 kDefaultGravity;

    World();
    ~World();

    const Vector3 gravity() const;
    void setGravity(const Vector3 &value);
    unsigned long randSeed() const;
    void setRandSeed(unsigned long value);
    void setMotionFPS(const Scalar &value);
    void addModel(vpvl2::IModel *value);
    void removeModel(vpvl2::IModel *value);
    void addRigidBody(btRigidBody *value);
    void removeRigidBody(btRigidBody *value);
    void stepSimulation(const Scalar &delta);
    btDiscreteDynamicsWorld *dynamicWorldRef() const;

private:
    btDefaultCollisionConfiguration m_config;
    QScopedPointer<btCollisionDispatcher> m_dispatcher;
    QScopedPointer<btDbvtBroadphase> m_broadphase;
    QScopedPointer<btSequentialImpulseConstraintSolver> m_solver;
    QScopedPointer<btDiscreteDynamicsWorld> m_world;
    Scalar m_motionFPS;
    Scalar m_maxSubSteps;
    Scalar m_fixedTimeStep;

    VPVL2_DISABLE_COPY_AND_ASSIGN(World)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif // WORLD_H
