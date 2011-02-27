/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

/* headers */

#include "MMDME/MMDME.h"

namespace MMDAI {

/* BulletPhysics::initialize: initialize BulletPhysics */
void BulletPhysics::initialize()
{
   m_collisionConfig = NULL;
   m_dispatcher = NULL;
   m_overlappingPairCache = NULL;
   m_solver = NULL;
   m_world = NULL;
   m_fps = 0;
}

/* BulletPhysics::clear: free BulletPhysics */
void BulletPhysics::clear()
{
   if (m_world) {
      /* release remaining objects within the world */
      int numObject = m_world->getNumCollisionObjects();
      for (int i = 0; i < numObject; i++) {
         btCollisionObject *obj = m_world->getCollisionObjectArray()[i];
         btRigidBody *body = btRigidBody::upcast(obj);
         if (body && body->getMotionState())
            delete body->getMotionState();
         m_world->removeCollisionObject(obj);
         delete obj;
      }
   }

   delete m_world;
   delete m_solver;
   delete m_overlappingPairCache;
   delete m_dispatcher;
   delete m_collisionConfig;

   initialize();
}

/* BulletPhysics::BulletPhysics: constructor */
BulletPhysics::BulletPhysics()
{
   initialize();
}

/* BulletPhysics::~BulletPhysics: destructor */
BulletPhysics::~BulletPhysics()
{
   clear();
}

/* BulletPhysics::setup: initialize and setup BulletPhysics */
void BulletPhysics::setup(int simulationFps)
{
   float dist = 400.0f;

   clear();

   /* store values */
   m_fps = simulationFps;
   m_subStep = btScalar(1.0 / btScalar(m_fps));

   /* make a collision configuration */
   m_collisionConfig = new btDefaultCollisionConfiguration();

   /* make a collision dispatcher from the configuration for sequenciall processing */
   m_dispatcher = new btCollisionDispatcher(m_collisionConfig);

   /* set broadphase */
   m_overlappingPairCache = new btAxisSweep3(btVector3(-dist, -dist, -dist), btVector3(dist, dist, dist), 1024);

   /* make a sequencial constraint solver */
   m_solver = new btSequentialImpulseConstraintSolver();

   /* create simulation world */
   m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfig);

   /* set default gravity */
   /* some tweak for the simulation to match that of MMD... */
   m_world->setGravity(btVector3(0.0f, -9.8f * 2, 0.0f));

   /* a weird configuration to use 120Hz simulation */
   /* change the number of constraint solving iteration to be inversely propotional to simulation rate */
   /* is this a bug of bulletphysics? */
   m_world->getSolverInfo().m_numIterations = (int) (10 * 60 / m_fps);
}

/* BulletPhysics::update: step the simulation world forward */
void BulletPhysics::update(float deltaFrame)
{
   btScalar sec = deltaFrame / 30.0f; /* convert frame to second */

   if (sec > 1.0) {
      /* long pause, just move ahead at one step */
      m_world->stepSimulation(sec, 1, sec);
   } else {
      /* progress by (1.0/fps) sub step */
      m_world->stepSimulation(sec, m_fps, m_subStep);
   }
}

/* BulletPhysics::getWorld: get simulation world */
btDiscreteDynamicsWorld *BulletPhysics::getWorld() const
{
   return m_world;
}

} /* namespace */

