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

#ifndef MMDAI_BULLETPHYSICS_H_
#define MMDAI_BULLETPHYSICS_H_

#define BULLETPHYSICS_PI 3.1415926535897932384626433832795

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <btBulletDynamicsCommon.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/* BulletPhysics: Bullet Physics engine */
class BulletPhysics
{
private:

   btDefaultCollisionConfiguration *m_collisionConfig; /* collision configuration */
   btCollisionDispatcher *m_dispatcher;                /* collision dispatcher */
   btAxisSweep3 *m_overlappingPairCache;
   btConstraintSolver *m_solver;                       /* constraint solver */
   btDiscreteDynamicsWorld *m_world;                   /* the simulation world */

   int m_fps;          /* simulation frame rate (Hz) */
   btScalar m_subStep; /* sub step to process simulation */

   GLuint m_boxList;         /* display list (box) */
   bool m_boxListEnabled;
   GLuint m_sphereList;      /* display list (sphere) */
   bool m_sphereListEnabled;

   /* initialize: initialize BulletPhysics */
   void initialize();

   /* clear: free BulletPhysics */
   void clear();

public:

   /* BulletPhysics: constructor */
   BulletPhysics();

   /* ~BulletPhysics: destructor */
   ~BulletPhysics();

   /* setup: initialize and setup BulletPhysics */
   void setup(int simulationFps);

   /* update: step the simulation world forward */
   void update(float deltaFrame);

   /* getWorld: get simulation world */
   btDiscreteDynamicsWorld *getWorld();

   void debugDisplay();
};

#endif

