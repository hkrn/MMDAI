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

#ifndef MMDME_PMDMODEL_H_
#define MMDME_PMDMODEL_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/Common.h"
#include "MMDME/BulletPhysics.h"
#include "MMDME/PMDBone.h"
#include "MMDME/PMDConstraint.h"
#include "MMDME/PMDFace.h"
#include "MMDME/PMDIK.h"
#include "MMDME/PMDMaterial.h"
#include "MMDME/PMDModelLoader.h"
#include "MMDME/PMDRigidBody.h"
#include "MMDME/PMDTexture.h"
#include "MMDME/PTree.h"

namespace MMDAI {

/* TexCoord: texture coordinaiton */
typedef struct {
   float u;
   float v;
} TexCoord;

/* PMDModel: model of PMD */
class PMDModel
{
public:
   static const int  kNSystemTextureFiles = 10;

private:

   /* model definition */
   char *m_name;     /* model name */
   char *m_comment;  /* comment string */

   unsigned int m_numVertex; /* number of vertices */
   btVector3 *m_vertexList;   /* vertex list */
   btVector3 *m_normalList;   /* normal list */
   TexCoord *m_texCoordList;  /* texture coordinate list */
   short *m_bone1List;        /* weighted bone ID list */
   short *m_bone2List;        /* weighted bone ID list */
   float *m_boneWeight1;      /* weight list for m_Bone1List */
   bool *m_noEdgeFlag;        /* true if no edge should be drawn for this vertex */

   unsigned int m_numSurface;    /* number of surface definitions */
   unsigned short *m_surfaceList; /* list of surface definitions (index to 3 vertices per surface) */

   unsigned int m_numMaterial; /* number of material definitions */
   PMDMaterial *m_material;     /* material list */

   unsigned short m_numBone; /* number of bones */
   PMDBone *m_boneList;      /* bone list */

   unsigned short m_numIK; /* number of IK chains */
   PMDIK *m_IKList;        /* IK chain list */

   unsigned short m_numFace; /* number of face definitions */
   PMDFace *m_faceList;      /* face definition list */

   unsigned int m_numRigidBody;  /* number of rigid bodies (Bullet Physics) */
   PMDRigidBody *m_rigidBodyList; /* rigid body list */

   unsigned int m_numConstraint;   /* number of constraints (Bullet Physics) */
   PMDConstraint *m_constraintList; /* rigid body list */

   /* work area for toon renderling */
   unsigned int m_toonTextureID[kNSystemTextureFiles + 1];  /* texture ID for toon shading */
   PMDTexture m_localToonTexture[kNSystemTextureFiles + 1]; /* toon textures for this model only */

   /* work area for OpenGL rendering */
   btTransform *m_boneSkinningTrans;         /* transform matrices of bones for skinning */
   btVector3 *m_skinnedVertexList;           /* vertex list after skinning */
   btVector3 *m_skinnedNormalList;           /* normal list after skinning */
   TexCoord *m_toonTexCoordList;             /* texture coordination list for toon shading */
   btVector3 *m_edgeVertexList;              /* vertex list for edge drawing */
   unsigned int m_numSurfaceForEdge;        /* number of edge-drawing surface list */
   unsigned short *m_surfaceListForEdge;     /* surface list on which toon edge will be drawn per material */
   TexCoord *m_toonTexCoordListForShadowMap; /* texture coordinates for toon shading on shadow mapping */

   /* flags and short lists extracted from the model data */
   PMDBone *m_centerBone;              /* center bone */
   PMDFace *m_baseFace;                /* base face definition */
   bool m_hasSingleSphereMap;            /* true if this model has Sphere map texture */
   bool m_hasMultipleSphereMap;          /* true if this model has additional sphere map texture */
   unsigned short m_numRotateBone;     /* number of bones under rotatation of other bone (type == 5 or 9) */
   unsigned short *m_rotateBoneIDList; /* ID list of under-rotate bones */
   bool *m_IKSimulated;                /* boolean list whether an IK should be disabled due to simulation */
   bool m_enableSimulation;            /* true when physics bone control is enabled and simulated IK should be skipped */
   float m_maxHeight;                  /* maximum height of this model */

   /* configuration parameters given from outside */
   bool m_toon;                   /* true when enable toon rendering */
   float m_globalAlpha;           /* global alpha value */
   float m_edgeOffset;            /* edge offset */
   bool m_selfShadowDrawing;      /* true when render with self shadow color */
   float m_selfShadowDensityCoef; /* shadow density coefficient for self shadow */
   float m_edgeColor[4];          /* edge color */

   /* additional information for managing model */
   BulletPhysics *m_bulletPhysics; /* pointer to BulletPhysics class, under which this model is controlled */
   PMDBone m_rootBone;             /* model root bone for global model offset / rotation / bone binding */
   PTree m_name2bone;              /* name-to-bone index for fast lookup */
   PTree m_name2face;              /* name-to-face index for fast lookup */

   /* parse: initialize and load from data memories */
   bool parse(PMDModelLoader *loader, BulletPhysics *bullet);

   /* initialize: initialize PMDModel */
   void initialize();

   /* clear: free PMDModel */
   void clear();

   MMDME_DISABLE_COPY_AND_ASSIGN(PMDModel);

public:

   static const float kMinBoneWeight;
   static const float kMinFaceWeight;

   static const float kEdgeColorR;
   static const float kEdgeColorG;
   static const float kEdgeColorB;
   static const float kEdgeColorA;

   /* PMDModel: constructor */
   PMDModel();

   /* ~PMDModel: destructor */
   ~PMDModel();

   /* load: load from file name */
   bool load(PMDModelLoader *loader, BulletPhysics *bullet);

   /* getBone: find bone data by name */
   PMDBone *getBone(const char *name);

   /* getFace: find face data by name */
   PMDFace *getFace(const char *name);

   /* getChildBoneList: return list of child bones, in decent order */
   int getChildBoneList(PMDBone **bone, unsigned short boneNum, PMDBone **childBoneList, unsigned short childBoneNumMax);

   /* setPhysicsControl switch bone control by physics simulation */
   void setPhysicsControl(bool flag);

   /* release: free PMDModel */
   void release();

   /* PMDModel:;setEdgeThin: set edge offset */
   void setEdgeThin(float thin);

   /* PMDModel:;setToonFlag: set toon rendering flag */
   void setToonFlag(bool flag);

   /* getToonFlag: return true when enable toon rendering */
   bool getToonFlag() const;

   /* setSelfShadowDrawing: set self shadow drawing flag */
   void setSelfShadowDrawing(bool flag);

   /* setEdgeColor: set edge color */
   void setEdgeColor(float col[4]);

   /* setGlobalAlpha: set global alpha value */
   void setGlobalAlpha(float alpha);

   /* getRootBone: get root bone */
   PMDBone *getRootBone();

   /* getCenterBone: get center bone */
   PMDBone *getCenterBone() const;

   /* getName: get model name */
   const char *getName() const;

   /* getNumVertex: get number of vertics */
   unsigned int getNumVertex() const;

   /* getNumSurface: get number of surface definitions */
   unsigned int getNumSurface() const;

   /* getNumMaterial: get number of material definitions */
   unsigned int getNumMaterial() const;

   /* getNumBone: get number of bones */
   unsigned short getNumBone() const;

   /* getNumIK: get number of IK chains */
   unsigned short getNumIK() const;

   /* getNumFace: get number of faces */
   unsigned short getNumFace() const;

   /* getNumRigidBody: get number of rigid bodies */
   unsigned int getNumRigidBody() const;

   /* getNumConstraint: get number of constraints */
   unsigned int getNumConstraint() const;

   /* getErrorTextureList: get error texture list */
   void getErrorTextureList(char *buf, int maxLen);

   /* getMaxHeight: get max height */
   float getMaxHeight() const;

   /* getComment: get comment of PMD */
   const char *getComment() const;

   /* updateBone: update bones */
   void updateBone();

   /* updateBoneFromSimulation: update bone transform from rigid body */
   void updateBoneFromSimulation();

   /* updateFace: update face morph from current face weights */
   void updateFace();

   /* updateSkin: update skin data from bone orientation */
   void updateSkin();

   /* updateToon: update toon coordinates and edge vertices */
   void updateToon(btVector3 *light);

   /* updateShadowColorTexCoord: update / create pseudo toon coordinates for shadow rendering pass on shadow mapping */
   void updateShadowColorTexCoord(float coef);

   /* calculateBoundingSphereRange: calculate the bounding sphere for depth texture rendering on shadow mapping */
   float calculateBoundingSphereRange(btVector3 *cpos);

   /* smearAllBonesToDefault: smear all bone pos/rot into default value (rate 1.0 = keep, rate 0.0 = reset) */
   void smearAllBonesToDefault(float rate);

   PMDBone **getBonesPtr();
   const btVector3 *getVerticesPtr() const;
   const btVector3 *getNormalsPtr() const;
   const TexCoord *getTexCoordsPtr() const;
   const btVector3 *getSkinnedVerticesPtr() const;
   const btVector3 *getSkinnedNormalsPtr() const;
   const TexCoord *getToonTexCoordsPtr() const;
   const TexCoord *getToonTexCoordsForSelfShadowPtr() const;
   const btVector3 *getEdgeVerticesPtr() const;
   const unsigned short *getSurfacesPtr() const;
   const unsigned short *getSurfacesForEdgePtr() const;
   const float getGlobalAlpha() const;
   const float *getEdgeColors() const;
   PMDMaterial *getMaterialAt(unsigned int i);
   const unsigned int getToonTextureIDAt(unsigned int i) const;
   const unsigned int getNumSurfaceForEdge() const;
   const bool isSelfShadowEnabled() const;
   const bool hasSingleSphereMap() const;
   const bool hasMultipleSphereMap() const;
};

} /* namespace */

#endif

