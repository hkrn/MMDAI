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

/* Bullet Physics */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#endif
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
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
          motionFPS(0),
          fixedTimeStep(0),
          maxSubSteps(0)
    {
        dispatcher = new btCollisionDispatcher(&config);
        broadphase = new btDbvtBroadphase();
        solver = new btSequentialImpulseConstraintSolver();
        world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
        world->getSolverInfo().m_solverMode &= ~SOLVER_RANDMIZE_ORDER;
    }
    ~PrivateContext() {
        internal::deleteObject(dispatcher);
        internal::deleteObject(broadphase);
        internal::deleteObject(solver);
        internal::deleteObject(world);
        motionFPS = 0;
        maxSubSteps = 0;
        fixedTimeStep = 0;
    }

    btDefaultCollisionConfiguration config;
    btCollisionDispatcher *dispatcher;
    btDbvtBroadphase *broadphase;
    btSequentialImpulseConstraintSolver *solver;
    btDiscreteDynamicsWorld *world;
    Scalar motionFPS;
    Scalar fixedTimeStep;
    int maxSubSteps;
};

const int World::kDefaultMaxSubSteps = 2;

World::World()
    : m_context(new PrivateContext())
{
    setGravity(vpvl2::Vector3(0.0f, -9.8f, 0.0f));
    setPreferredFPS(vpvl2::Scene::defaultFPS());
    setMaxSubSteps(kDefaultMaxSubSteps);
}

World::~World()
{
    internal::deleteObject(m_context);
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

unsigned long World::randSeed() const
{
    return m_context->solver->getRandSeed();
}

Scalar World::motionFPS() const
{
    return m_context->motionFPS;
}

Scalar World::fixedTimeStep() const
{
    return m_context->fixedTimeStep;
}

int World::maxSubSteps() const
{
    return m_context->maxSubSteps;
}

void World::setRandSeed(unsigned long value)
{
    m_context->solver->setRandSeed(value);
}

void World::setPreferredFPS(const Scalar &value)
{
    m_context->motionFPS = value;
    m_context->fixedTimeStep = 1.0f / value;
}

void World::setMaxSubSteps(int value)
{
    m_context->maxSubSteps = value;
}

void World::addRigidBody(btRigidBody *value)
{
    m_context->world->addRigidBody(value);
}

void World::removeRigidBody(btRigidBody *value)
{
    m_context->world->removeRigidBody(value);
}

void World::stepSimulation(const vpvl2::Scalar &delta)
{
    m_context->world->stepSimulation(delta, m_context->maxSubSteps, m_context->fixedTimeStep);
}

} /* namespace extensions */
} /* namespace vpvl2 */
