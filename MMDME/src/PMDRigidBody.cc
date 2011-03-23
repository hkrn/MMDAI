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

#include "AlignedMotionState.h"
#include "KinematicMotionState.h"

namespace MMDAI {

/* PMDRigidBody::initialize: initialize PMDRigidBody */
void PMDRigidBody::initialize()
{
   m_shape = NULL;
   m_body = NULL;
   m_motionState = NULL;
   m_groupID = 0;
   m_groupMask = 0;

   m_type = 0;
   m_bone = NULL;
   m_noBone = false;
   m_kinematicMotionState = NULL;

   m_world = NULL;
}

/* PMDRigidBody::clear: free PMDRigidBody */
void PMDRigidBody::clear()
{
   /* release motion state */
   if (m_motionState)
      delete m_motionState;
   if (m_kinematicMotionState)
      delete m_kinematicMotionState;
   if (m_body) {
      m_world->removeCollisionObject(m_body); /* release body */
      delete m_body;
   }
   if (m_shape)
      delete m_shape;

   initialize();
}

/* PMDRigidBody::PMDRigidBody: constructor */
PMDRigidBody::PMDRigidBody()
{
   initialize();
}

/* PMDRigidBody::~PMDRigidBody: destructor */
PMDRigidBody::~PMDRigidBody()
{
   clear();
}

/* PMDRigidBody::setup: initialize and setup PMDRigidBody */
bool PMDRigidBody::setup(PMDFile_RigidBody *rb, PMDBone *bone)
{
   btScalar mass;
   btVector3 localInertia(0.0f, 0.0f, 0.0f);
   btMatrix3x3 bm;
   btTransform startTrans;

   clear();

   /* store bone */
   m_bone = bone;
   m_noBone = false;
   if (rb->boneID == 0xFFFF)
      m_noBone = true;

   /* store values*/
   m_type = rb->type;

   /* create shape */
   if (rb->shapeType == 0) {
      /* sphere: radius == width */
      m_shape = new btSphereShape(rb->width);
   } else if (rb->shapeType == 1) {
      /* box: half extent: width, height, depth */
      m_shape = new btBoxShape(btVector3(rb->width, rb->height, rb->depth));
   } else if (rb->shapeType == 2) {
      m_shape = new btCapsuleShape(rb->width, rb->height);
   } else {
      return false;
   }

   /* set mass and local inertial tensor */
   if (rb->type != 0)
      mass = rb->mass; /* dynamic (non-kinematic) bodies */
   else
      mass = 0.0f; /* the mass of static (kinematic) bodies should be always set to 0 */
   if (mass != 0.0f)
      m_shape->calculateLocalInertia(mass, localInertia);

   /* set position and rotation of the rigid body, local to the associated bone */
   m_trans.setIdentity();
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   btMatrix3x3 rx, ry, rz;
   rx.setEulerZYX(-rb->rot[0], 0, 0);
   ry.setEulerZYX(0, -rb->rot[1], 0);
   rz.setEulerZYX(0, 0, rb->rot[2]);
   bm = ry * rz * rx;
#else
   bm.setEulerZYX(rb->rot[0], rb->rot[1], rb->rot[2]);
#endif
   m_trans.setBasis(bm);
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   m_trans.setOrigin(btVector3(rb->pos[0], rb->pos[1], -rb->pos[2]));
#else
   m_trans.setOrigin(btVector3(rb->pos[0], rb->pos[1], rb->pos[2]));
#endif

   /* calculate initial global transform */
   startTrans.setIdentity();
   startTrans.setOrigin(m_bone->getTransform()->getOrigin());
   startTrans *= m_trans;

   /* prepare motion state */
   if (rb->type == 0) {
      /* kinematic body, will be moved along the motion of corresponding bone */
      m_motionState = new KinematicMotionState(startTrans, m_trans, m_bone);
      m_kinematicMotionState = NULL;
   } else if (rb->type == 1) {
      /* simulated body, use default motion state */
      m_motionState = new btDefaultMotionState(startTrans);
      m_kinematicMotionState = new KinematicMotionState(startTrans, m_trans, m_bone);
   } else {
      /* simulated body, will be aligned to the motion-controlled bone */
      m_motionState = new AlignedMotionState(startTrans, m_trans, m_bone);
      m_kinematicMotionState = new KinematicMotionState(startTrans, m_trans, m_bone);
   }

   /* set rigid body parameters */
   btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_motionState, m_shape, localInertia);
   rbInfo.m_linearDamping = rb->linearDamping;
   rbInfo.m_angularDamping = rb->angularDamping;
   rbInfo.m_restitution = rb->restitution;
   rbInfo.m_friction = rb->friction;
   /* additional damping can help avoiding lowpass jitter motion, help stability for ragdolls etc. */
   rbInfo.m_additionalDamping = true;

   /* make rigid body for the shape */
   m_body = new btRigidBody(rbInfo);

   /* for knematic body, flag them as kinematic and disable the sleeping/deactivation */
   if (rb->type == 0)
      m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);

   /* disable deactivation for all rigid bodies */
   m_body->setActivationState(DISABLE_DEACTIVATION);

   /* set collision group index and group mask */
   m_groupID = 0x0001 << rb->collisionGroupID;
   m_groupMask = rb->collisionMask;

   /* store inverse matrix of local transform */
   m_transInv = m_trans.inverse();

   MMDAILogDebugSJIS("name=%s", rb->name);

   return true;
}

/* PMDRigidBody::joinWorld: add the body to simulation world */
void PMDRigidBody::joinWorld(btDiscreteDynamicsWorld *btWorld)
{
   if (! m_body) return;

   /* add the body to the simulation world, with group id and group mask for collision */
   btWorld->addRigidBody(m_body, m_groupID, m_groupMask);
   m_world = btWorld;
}

/* PMDRigidBody::applyTransformToBone: apply the current rigid body transform to bone after simulation (for type 1 and 2) */
void PMDRigidBody::applyTransformToBone()
{
   btTransform tr;

   if (m_type == 0 || m_bone == NULL || m_noBone) return;

   tr = m_body->getCenterOfMassTransform();
   tr *= m_transInv;
   m_bone->setTransform(&tr);
}

/* PMDRigidBody::setKinematic: switch between Default and Kinematic body for non-simulated movement */
void PMDRigidBody::setKinematic(bool flag)
{
   btTransform tr;

   if (m_type == 0) return; /* always kinematic */

   if (flag) {
      /* default to kinematic */
      m_body->setMotionState(m_kinematicMotionState);
      m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
   } else {
      /* kinematic to default */
      m_kinematicMotionState->getWorldTransform(tr);
      m_motionState->setWorldTransform(tr);
      m_body->setMotionState(m_motionState);
      m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
   }
}

/* PMDRigidBody::getBody: get rigid body */
btRigidBody *PMDRigidBody::getBody() const
{
   return m_body;
}

} /* namespace */

