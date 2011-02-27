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

const float PMDModel::kMinBoneWeight = 0.0001f;
const float PMDModel::kMinFaceWeight = 0.001f;

const float PMDModel::kEdgeColorR = 0.0f;
const float PMDModel::kEdgeColorG = 0.0f;
const float PMDModel::kEdgeColorB = 0.0f;
const float PMDModel::kEdgeColorA = 1.0f;

/* PMDModel::initialize: initialize PMDModel */
void PMDModel::initialize()
{
   m_name = NULL;
   m_comment = NULL;
   m_bulletPhysics = NULL;

   m_vertexList = NULL;
   m_normalList = NULL;
   m_texCoordList = NULL;
   m_bone1List = NULL;
   m_bone2List = NULL;
   m_boneWeight1 = NULL;
   m_noEdgeFlag = NULL;
   m_surfaceList = NULL;
   m_material = NULL;
   m_boneList = NULL;
   m_IKList = NULL;
   m_faceList = NULL;
   m_rigidBodyList = NULL;
   m_constraintList = NULL;

   m_boneSkinningTrans = NULL;
   m_skinnedVertexList = NULL;
   m_skinnedNormalList = NULL;
   m_toonTexCoordList = NULL;
   m_edgeVertexList = NULL;
   m_surfaceListForEdge = NULL;
   m_toonTexCoordListForShadowMap = NULL;
   m_rotateBoneIDList = NULL;
   m_IKSimulated = NULL;

   /* initial values for variables that should be kept at model change */
   m_enableSimulation = true;
   m_toon = false;
   m_globalAlpha = 1.0f;
   m_edgeOffset = 0.03f;
   m_selfShadowDrawing = false;
   m_selfShadowDensityCoef = 0.0f;
   m_edgeColor[0] = kEdgeColorR;
   m_edgeColor[1] = kEdgeColorG;
   m_edgeColor[2] = kEdgeColorB;
   m_edgeColor[3] = kEdgeColorA;
   m_rootBone.reset();
}

/* PMDModel::clear: free PMDModel */
void PMDModel::clear()
{
   int i;

   if (m_vertexList) {
      delete [] m_vertexList;
      m_vertexList = NULL;
   }
   if (m_normalList) {
      delete [] m_normalList;
      m_normalList = NULL;
   }
   if (m_texCoordList) {
      MMDAIMemoryRelease(m_texCoordList);
      m_texCoordList = NULL;
   }
   if (m_bone1List) {
      MMDAIMemoryRelease(m_bone1List);
      m_bone1List = NULL;
   }
   if (m_bone2List) {
      MMDAIMemoryRelease(m_bone2List);
      m_bone2List = NULL;
   }
   if (m_boneWeight1) {
      MMDAIMemoryRelease(m_boneWeight1);
      m_boneWeight1 = NULL;
   }
   if (m_noEdgeFlag) {
      MMDAIMemoryRelease(m_noEdgeFlag);
      m_noEdgeFlag = NULL;
   }
   if (m_surfaceList) {
      MMDAIMemoryRelease(m_surfaceList);
      m_surfaceList = NULL;
   }
   if (m_material) {
      delete [] m_material;
      m_material = NULL;
   }
   if (m_boneList) {
      delete [] m_boneList;
      m_boneList = NULL;
   }
   if (m_IKList) {
      delete [] m_IKList;
      m_IKList = NULL;
   }
   if (m_faceList) {
      delete [] m_faceList;
      m_faceList = NULL;
   }
   if (m_constraintList) {
      delete [] m_constraintList;
      m_constraintList = NULL;
   }
   if (m_rigidBodyList) {
      delete [] m_rigidBodyList;
      m_rigidBodyList = NULL;
   }

   if (m_boneSkinningTrans) {
      delete [] m_boneSkinningTrans;
      m_boneSkinningTrans = NULL;
   }
   if (m_skinnedVertexList) {
      delete [] m_skinnedVertexList;
      m_skinnedVertexList = NULL;
   }
   if (m_skinnedNormalList) {
      delete [] m_skinnedNormalList;
      m_skinnedNormalList = NULL;
   }
   if (m_toonTexCoordList) {
      MMDAIMemoryRelease(m_toonTexCoordList);
      m_toonTexCoordList = NULL;
   }
   if (m_edgeVertexList) {
      delete [] m_edgeVertexList;
      m_edgeVertexList = NULL;
   }
   if (m_surfaceListForEdge) {
      MMDAIMemoryRelease(m_surfaceListForEdge);
      m_surfaceListForEdge = NULL;
   }
   if (m_toonTexCoordListForShadowMap) {
      MMDAIMemoryRelease(m_toonTexCoordListForShadowMap);
      m_toonTexCoordListForShadowMap = NULL;
   }
   if (m_rotateBoneIDList) {
      MMDAIMemoryRelease(m_rotateBoneIDList);
      m_rotateBoneIDList = NULL;
   }
   if (m_IKSimulated) {
      MMDAIMemoryRelease(m_IKSimulated);
      m_IKSimulated = NULL;
   }
   if(m_comment) {
      MMDAIMemoryRelease(m_comment);
      m_comment = NULL;
   }
   if(m_name) {
      MMDAIMemoryRelease(m_name);
      m_name = NULL;
   }

   for (i = 0; i < kNSystemTextureFiles; i++)
      m_localToonTexture[i].release();
   m_name2bone.release();
   m_name2face.release();
}

/* PMDModel::PMDModel: constructor */
PMDModel::PMDModel()
{
   initialize();
}

/* PMDModel::~PMDModel: destructor */
PMDModel::~PMDModel()
{
   clear();
}

/* PMDModel::load: load from file name */
bool PMDModel::load(PMDModelLoader *loader, PMDRenderEngine *engine, BulletPhysics *bullet)
{
   bool ret = parse(loader, engine, bullet);
   return ret;
}

/* PMDModel::getBone: find bone data by name */
PMDBone *PMDModel::getBone(const char *name) 
{
   PMDBone *match = static_cast<PMDBone *>(m_name2bone.findNearest(name));

   if (match && MMDAIStringEquals(match->getName(), name))
      return match;
   else
      return NULL;
}

/* PMDModel::getFace: find face data by name */
PMDFace *PMDModel::getFace(const char *name)
{
   PMDFace *match = static_cast<PMDFace *>(m_name2face.findNearest(name));

   if (match && MMDAIStringEquals(match->getName(), name))
      return match;
   else
      return NULL;
}

/* PMDModel::getChildBoneList: return list of child bones, in decent order */
int PMDModel::getChildBoneList(PMDBone **bone, unsigned short boneNum, PMDBone **childBoneList, unsigned short childBoneNumMax)
{
   unsigned short i, j;
   PMDBone *b;
   int n, k, l;
   bool updated;
   int iFrom, iTo;

   for (i = 0, n = 0; i < boneNum; i++) {
      b = bone[i];
      for (j = 0; j < m_numBone; j++) {
         if (m_boneList[j].getParentBone() == b) {
            if (n >= childBoneNumMax)
               return -1;
            childBoneList[n] = &(m_boneList[j]);
            n++;
         }
      }
   }

   updated = true;
   iFrom = 0;
   iTo = n;
   while (updated) {
      updated = false;
      for (k = iFrom; k < iTo; k++) {
         b = childBoneList[k];
         for (j = 0; j < m_numBone; j++) {
            if (m_boneList[j].getParentBone() == b) {
               for (l = 0; l < n; l++)
                  if (childBoneList[l] == &(m_boneList[j]))
                     break;
               if (l < n)
                  continue;
               if (n >= childBoneNumMax)
                  return -1;
               childBoneList[n] = &(m_boneList[j]);
               n++;
               updated = true;
            }
         }
      }
      iFrom = iTo;
      iTo = n;
   }

   return n;
}

/* PMDModel::setPhysicsControl switch bone control by physics simulation */
void PMDModel::setPhysicsControl(bool flag)
{
   unsigned int i;

   m_enableSimulation = flag;
   /* when true, align all rigid bodies to corresponding bone by setting Kinematics flag */
   /* when false, all rigid bodies will have their own motion states according to the model definition */
   for (i = 0; i < m_numRigidBody; i++)
      m_rigidBodyList[i].setKinematic(!flag);
}

/* PMDModel::release: free PMDModel */
void PMDModel::release()
{
   clear();
}

/* PMDModel:;setEdgeThin: set edge offset */
void PMDModel::setEdgeThin(float thin)
{
   m_edgeOffset = thin * 0.03f;
}

/* PMDModel:;setToonFlag: set toon rendering flag */
void PMDModel::setToonFlag(bool flag)
{
   m_toon = flag;
}

/* PMDModel::getToonFlag: return true when enable toon rendering */
bool PMDModel::getToonFlag() const
{
   return m_toon;
}

/* PMDModel::setSelfShadowDrawing: set self shadow drawing flag */
void PMDModel::setSelfShadowDrawing(bool flag)
{
   m_selfShadowDrawing = flag;
}

/* PMDModel::setEdgeColor: set edge color */
void PMDModel::setEdgeColor(float col[4])
{
   int i;

   for (i = 0; i < 4; i++)
      m_edgeColor[i] = col[i];
}

/* PMDModel::setGlobalAlpha: set global alpha value */
void PMDModel::setGlobalAlpha(float alpha)
{
   m_globalAlpha = alpha;
}

/* PMDModel::getRootBone: get root bone */
PMDBone *PMDModel::getRootBone()
{
   return &m_rootBone;
}

/* PMDModel::getCenterBone: get center bone */
PMDBone *PMDModel::getCenterBone() const
{
   return m_centerBone;
}

/* PMDModel::getName: get name */
const char *PMDModel::getName() const
{
   return m_name;
}

/* PMDModel::getNumVertex: get number of vertics */
unsigned int PMDModel::getNumVertex() const
{
   return m_numVertex;
}

/* PMDModel::getNumSurface: get number of surface definitions */
unsigned int PMDModel::getNumSurface() const
{
   return m_numSurface;
}

/* PMDModel::getNumMaterial: get number of material definitions */
unsigned int PMDModel::getNumMaterial() const
{
   return m_numMaterial;
}

/* PMDModel::getNumBone: get number of bones */
unsigned short PMDModel::getNumBone() const
{
   return m_numBone;
}

/* PMDModel::getNumIK: get number of IK chains */
unsigned short PMDModel::getNumIK() const
{
   return m_numIK;
}

/* PMDModel::getNumFace: get number of faces */
unsigned short PMDModel::getNumFace() const
{
   return m_numFace;
}

/* PMDModel::getNumRigidBody: get number of rigid bodies */
unsigned int PMDModel::getNumRigidBody() const
{
   return m_numRigidBody;
}

/* PMDModel::getNumConstraint: get number of constraints */
unsigned int PMDModel::getNumConstraint() const
{
   return m_numConstraint;
}

/* PMDModel::getErrorTextureList: get error texture list */
void PMDModel::getErrorTextureList(char *buf, int maxLen)
{
   (void)buf;
   (void)maxLen;
   //m_textureLoader.getErrorTextureString(buf, maxLen);
}

/* PMDModel::getMaxHeight: get max height */
float PMDModel::getMaxHeight() const
{
   return m_maxHeight;
}

/* PMDModel::getComment: get comment of PMD */
const char *PMDModel::getComment() const
{
   return m_comment;
}

PMDBone *PMDModel::getBonesPtr() const
{
  return m_boneList;
}

const btVector3 *PMDModel::getVerticesPtr() const
{
  return m_vertexList;
}

const btVector3 *PMDModel::getNormalsPtr() const
{
  return m_normalList;
}

const TexCoord *PMDModel::getTexCoordsPtr() const
{
  return m_texCoordList;
}

const btVector3 *PMDModel::getSkinnedVerticesPtr() const
{
  return m_skinnedVertexList;
}

const btVector3 *PMDModel::getSkinnedNormalsPtr() const
{
  return m_skinnedNormalList;
}

const TexCoord *PMDModel::getToonTexCoordsPtr() const
{
  return m_toonTexCoordList;
}

const TexCoord *PMDModel::getToonTexCoordsForSelfShadowPtr() const
{
  return m_toonTexCoordListForShadowMap;
}

const btVector3 *PMDModel::getEdgeVerticesPtr() const
{
  return m_edgeVertexList;
}

const unsigned short *PMDModel::getSurfacesPtr() const
{
  return m_surfaceList;
}

const unsigned short *PMDModel::getSurfacesForEdgePtr() const
{
  return m_surfaceListForEdge;
}

const float PMDModel::getGlobalAlpha() const
{
  return m_globalAlpha;
}

const float *PMDModel::getEdgeColors() const
{
  return m_edgeColor;
}

PMDMaterial *PMDModel::getMaterialAt(unsigned int i)
{
  if ( i >= m_numMaterial)
    return NULL;
  else
    return &m_material[i];
}

PMDTexture *PMDModel::getToonTextureAt(unsigned int i)
{
  if (i >= kNSystemTextureFiles + 1)
    return 0;
  else
    return &m_localToonTexture[i];
}

const unsigned int PMDModel::getNumSurfaceForEdge() const
{
  return m_numSurfaceForEdge;
}

const bool PMDModel::isSelfShadowEnabled() const
{
  return m_selfShadowDrawing;
}

const bool PMDModel::hasSingleSphereMap() const
{
  return m_hasSingleSphereMap;
}

const bool PMDModel::hasMultipleSphereMap() const
{
  return m_hasMultipleSphereMap;
}

} /* namespace */

