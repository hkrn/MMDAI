/**

 Copyright (c) 2010-2014  hkrn

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

#include <vpvl2/extensions/World.h>

#include <vpvl2/IModel.h>
#include <vpvl2/Scene.h>
#include <vpvl2/internal/util.h>

/* std::numeric_limits */
#include <limits>

/* Bullet Physics */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#endif
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace vpvl2
{
namespace extensions
{

struct World::PrivateContext {
    PrivateContext()
        : dispatcher(0),
          broadphase(0),
          solver(0),
          world(0),
          ground(0),
          groundBody(0),
          baseFPS(60.0f),
          timeScale(1.0f),
          enableFloor(true)
    {
        dispatcher = new btCollisionDispatcher(&config);
        broadphase = new btDbvtBroadphase();
        solver = new btSequentialImpulseConstraintSolver();
        world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
        world->getSolverInfo().m_solverMode &= ~SOLVER_RANDMIZE_ORDER;
        ground = new btStaticPlaneShape(Vector3(0, 1, 0), 0);
        btRigidBody::btRigidBodyConstructionInfo info(0, 0, ground, kZeroV3);
        groundBody = new btRigidBody(info);
        world->addRigidBody(groundBody, 0x10, 0);
    }
    ~PrivateContext() {
        world->removeRigidBody(groundBody);
        internal::deleteObject(groundBody);
        internal::deleteObject(ground);
        internal::deleteObject(dispatcher);
        internal::deleteObject(broadphase);
        internal::deleteObject(solver);
        internal::deleteObject(world);
        baseFPS = 0;
        timeScale = 0;
        enableFloor = false;
    }

    btDefaultCollisionConfiguration config;
    btCollisionDispatcher *dispatcher;
    btDbvtBroadphase *broadphase;
    btSequentialImpulseConstraintSolver *solver;
    btDiscreteDynamicsWorld *world;
    btStaticPlaneShape *ground;
    btRigidBody *groundBody;
    Scalar baseFPS;
    Scalar timeScale;
    bool enableFloor;
};

const int World::kDefaultMaxSubSteps = 2;

World::World()
    : m_context(new PrivateContext())
{
    setGravity(vpvl2::Vector3(0.0f, -9.8f, 0.0f));
}

World::~World()
{
    internal::deleteObject(m_context);
}

void World::addRigidBody(btRigidBody *value)
{
    m_context->world->addRigidBody(value);
}

void World::removeRigidBody(btRigidBody *value)
{
    m_context->world->removeRigidBody(value);
}

void World::deleteAll()
{
    btDiscreteDynamicsWorld *world = m_context->world;
    const int numCollidables = world->getNumCollisionObjects();
    for (int i = numCollidables - 1; i >= 0; i--) {
        btCollisionObject *object = world->getCollisionObjectArray().at(i);
        if (object != m_context->groundBody) {
            if (btRigidBody *body = btRigidBody::upcast(object)) {
                world->removeRigidBody(body);
                delete body->getMotionState();
            }
            else {
                world->removeCollisionObject(object);
            }
            delete object->getCollisionShape();
            delete object;
        }
    }
}

void World::stepSimulation(const vpvl2::Scalar &deltaTimeIndex, const vpvl2::Scalar &motionFPS)
{
    const Scalar &v = (deltaTimeIndex / motionFPS) * (m_context->baseFPS / motionFPS) * m_context->timeScale;
    m_context->world->stepSimulation(v, std::numeric_limits<int>::max(), 1.0f / m_context->baseFPS);
}

const vpvl2::Vector3 World::gravity() const
{
    return m_context->world->getGravity();
}

btDiscreteDynamicsWorld *World::dynamicWorldRef() const
{
    return m_context->world;
}

void World::setGravity(const vpvl2::Vector3 &value)
{
    m_context->world->setGravity(value);
}

Scalar World::baseFPS() const
{
    return m_context->baseFPS;
}

void World::setBaseFPS(const Scalar &value)
{
    m_context->baseFPS = value;
}

Scalar World::timeScale() const
{
    return m_context->timeScale;
}

void World::setTimeScale(const Scalar &value)
{
    m_context->timeScale = value;
}

unsigned long World::randSeed() const
{
    return m_context->solver->getRandSeed();
}

void World::setRandSeed(unsigned long value)
{
    m_context->solver->setRandSeed(value);
}

bool World::isFloorEnabled() const
{
    return m_context->enableFloor;
}

void World::setFloorEnabled(bool value)
{
    if (value) {
        m_context->world->removeRigidBody(m_context->groundBody);
        m_context->world->addRigidBody(m_context->groundBody, 0x10, 0);
    }
    else {
        m_context->world->removeRigidBody(m_context->groundBody);
    }
    m_context->enableFloor = value;
}


} /* namespace extensions */
} /* namespace vpvl2 */
