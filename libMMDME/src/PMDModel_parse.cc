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

#include "MMDME/MotionController.h"
#include "MMDME/PMDModel.h"
#include "MMDME/PMDTexture.h"
#include "MMDME/PMDModelLoader.h"

#include "PMDInternal.h"

/* PMDModel::parse: initialize and load from ptr memories */
bool PMDModel::parse(PMDModelLoader *loader, BulletPhysics *bullet)
{
   bool ret = true;
   btQuaternion defaultRot;

   PMDFile_Vertex *fileVertex = NULL;
   PMDFile_Material *fileMaterial = NULL;
   PMDFile_Bone *fileBone = NULL;
   unsigned char numFaceDisp = 0;
   unsigned char numBoneFrameDisp = 0;
   unsigned int numBoneDisp = 0;

   size_t size = 0;
   unsigned char *data = NULL;
   if (!loader->loadModelData(&data, &size))
      return false;

   unsigned char *start = data, *ptr = data;

   /* release internal variables */
   release();
   m_hasSingleSphereMap = false;
   m_hasMultipleSphereMap = false;
   m_baseFace = NULL;
   m_centerBone = NULL;

   /* reset root bone's rotation */
   defaultRot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
   m_rootBone.setCurrentRotation(&defaultRot);
   m_rootBone.update();

   m_bulletPhysics = bullet;

   /* reset toon texture IDs by system default textures */
   for (int i = 0; i < kNSystemTextureFiles; i++) {
      PMDTexture *texture = &m_localToonTexture[i];
      m_toonTextureID[i] = 0;
      if (loader->loadSystemTexture(i, texture)) {
         m_toonTextureID[i] = texture->getID();
      }
   }

   /* header */
   PMDFile_Header *fileHeader = (PMDFile_Header *) ptr;
   if (fileHeader->magic[0] != 'P' || fileHeader->magic[1] != 'm' || fileHeader->magic[2] != 'd')
      goto error;
   if (fileHeader->version != 1.0f)
      goto error;
   /* name */
   m_name = static_cast<char *>(MMDAIMemoryAllocate(sizeof(char) * (20 + 1)));
   if (m_name == NULL)
     goto error;
   MMDAIStringCopy(m_name, fileHeader->name, 20);
   m_name[20] = '\0';

   /* comment */
   m_comment = static_cast<char *>(MMDAIMemoryAllocate(sizeof(char) * (256 + 1)));
   if (m_comment == NULL)
     goto error;
   MMDAIStringCopy(m_comment, fileHeader->comment, 256);
   m_comment[256] = '\0';
   ptr += sizeof(PMDFile_Header);

   /* vertex ptr and bone weights */
   /* relocate as separated list for later OpenGL calls */
   m_numVertex = *((unsigned int *) ptr);
   ptr += sizeof(unsigned int);
   m_vertexList = new btVector3[m_numVertex];
   m_normalList = new btVector3[m_numVertex];
   m_texCoordList = static_cast<TexCoord *>(MMDAIMemoryAllocate(sizeof(TexCoord) * m_numVertex));
   if (m_texCoordList == NULL)
     goto error;
   m_bone1List = static_cast<short *>(MMDAIMemoryAllocate(sizeof(short) * m_numVertex));
   if (m_bone1List == NULL)
     goto error;
   m_bone2List = static_cast<short *>(MMDAIMemoryAllocate(sizeof(short) * m_numVertex));
   if (m_bone2List == NULL)
     goto error;
   m_boneWeight1 = static_cast<float *>(MMDAIMemoryAllocate(sizeof(float) * m_numVertex));
   if (m_boneWeight1 == NULL)
     goto error;
   m_noEdgeFlag = static_cast<bool *>(MMDAIMemoryAllocate(sizeof(bool) * m_numVertex));
   if (m_noEdgeFlag == NULL)
     goto error;

   fileVertex  = (PMDFile_Vertex *) ptr;
   for (unsigned int i = 0; i < m_numVertex; i++) {
      PMDFile_Vertex *fv = &fileVertex[i];
      m_vertexList[i].setValue(fv->pos[0], fv->pos[1], fv->pos[2]);
      m_normalList[i].setValue(fv->normal[0], fv->normal[1], fv->normal[2]);
      m_texCoordList[i].u = fv->uv[0];
      m_texCoordList[i].v = fv->uv[1];
      m_bone1List[i] = fv->boneID[0];
      m_bone2List[i] = fv->boneID[1];
      m_boneWeight1[i] = fv->boneWeight1 * 0.01f;
      m_noEdgeFlag[i] = fv->noEdgeFlag;
   }
   ptr += sizeof(PMDFile_Vertex) * m_numVertex;

   /* surface ptr, 3 vertex indices for each */
   m_numSurface = *((unsigned int *) ptr);
   ptr += sizeof(unsigned int);
   m_surfaceList = static_cast<unsigned short *>(MMDAIMemoryAllocate(sizeof(unsigned short) * m_numSurface));
   if (m_surfaceList == NULL)
     goto error;
   memcpy(m_surfaceList, ptr, sizeof(unsigned short) * m_numSurface);
   ptr += sizeof(unsigned short) * m_numSurface;

   /* material ptr (color, texture, toon parameter, edge flag) */
   m_numMaterial = *((unsigned int *) ptr);
   ptr += sizeof(unsigned int);
   m_material = new PMDMaterial[m_numMaterial];
   fileMaterial = (PMDFile_Material *) ptr;
   for (unsigned int i = 0; i < m_numMaterial; i++) {
      if (!m_material[i].setup(&fileMaterial[i], loader)) {
         /* ret = false; */
      }
   }
   ptr += sizeof(PMDFile_Material) * m_numMaterial;

   /* bone ptr */
   m_numBone = *((unsigned short *) ptr);
   ptr += sizeof(unsigned short);
   m_boneList = new PMDBone[m_numBone];
   fileBone = (PMDFile_Bone *) ptr;
   for (unsigned int i = 0; i < m_numBone; i++) {
      PMDBone *bone = &m_boneList[i];
      if (!bone->setup(&(fileBone[i]), m_boneList, m_numBone, &m_rootBone))
         ret = false;
      if (MMDAIStringEquals(bone->getName(), kCenterBoneName))
         m_centerBone = bone;
   }
   if (!m_centerBone && m_numBone >= 1) {
      /* if no bone is named "center," use the first bone as center */
      m_centerBone = &(m_boneList[0]);
   }
   ptr += sizeof(PMDFile_Bone) * m_numBone;
   /* calculate bone offset after all bone positions are loaded */
   for (unsigned int i = 0; i < m_numBone; i++)
      m_boneList[i].computeOffset();

   /* IK ptr */
   m_numIK = *((unsigned short *) ptr);
   ptr += sizeof(unsigned short);
   if (m_numIK > 0) {
      m_IKList = new PMDIK[m_numIK];
      for (unsigned int i = 0; i < m_numIK; i++) {
         PMDFile_IK *fileIK = (PMDFile_IK *) ptr;
         ptr += sizeof(PMDFile_IK);
         m_IKList[i].setup(fileIK, (short *)ptr, m_boneList);
         ptr += sizeof(short) * fileIK->numLink;
      }
   }

   /* face ptr */
   m_numFace = *((unsigned short *) ptr);
   ptr += sizeof(unsigned short);
   if (m_numFace > 0) {
      m_faceList = new PMDFace[m_numFace];
      for (unsigned int i = 0; i < m_numFace; i++) {
         PMDFace *face = &m_faceList[i];
         PMDFile_Face *fileFace = (PMDFile_Face *)ptr;
         ptr += sizeof(PMDFile_Face);
         face->setup(fileFace, (PMDFile_Face_Vertex *) ptr);
         if (fileFace->type == PMD_FACE_BASE)
            m_baseFace = face; /* store base face */
         ptr += sizeof(PMDFile_Face_Vertex) * fileFace->numVertex;
      }
      if (m_baseFace == NULL) {
         ret = false;
      } else {
         /* convert base-relative index to the index of original vertices */
         for (unsigned int i = 0; i < m_numFace; i++)
            m_faceList[i].convertIndex(m_baseFace);
      }
   }

   /* display names (skip) */
   /* indices for faces which should be displayed in "face" region */
   numFaceDisp = *((unsigned char *) ptr);
   ptr += sizeof(unsigned char) + sizeof(unsigned short) * numFaceDisp;
   /* bone frame names */
   numBoneFrameDisp = *((unsigned char *) ptr);
   ptr += sizeof(unsigned char) + 50 * numBoneFrameDisp;
   /* indices for bones which should be displayed in each bone region */
   numBoneDisp = *((unsigned int *) ptr);
   ptr += sizeof(unsigned int) + (sizeof(short) + sizeof(unsigned char)) * numBoneDisp;

   /* end of base format */
   /* check for remaining ptr */
   if ((unsigned long) ptr - (unsigned long) start >= size) {
      /* no extension ptr remains */
      m_numRigidBody = 0;
      m_numConstraint = 0;
      /* assign default toon textures for toon shading */
      for (int i = 0; i <= kNSystemTextureFiles; i++) {
        PMDTexture *texture = &m_localToonTexture[i];
        m_toonTextureID[i] = 0;
        if (loader->loadSystemTexture(i, texture)) {
          m_toonTextureID[i] = texture->getID();
        }
      }
   } else {
      /* English display names (skip) */
      unsigned char englishNameExist = *((unsigned char *) ptr);
      ptr += sizeof(unsigned char);
      if (englishNameExist != 0) {
         /* model name and comments in English */
         ptr += 20 + 256;
         /* bone names in English */
         ptr += 20 * m_numBone;
         /* face names in English */
         if (m_numFace > 0) ptr += 20 * (m_numFace - 1); /* "base" not included in English list */
         /* bone frame names in English */
         ptr += 50 * numBoneFrameDisp;
      }

      /* toon texture file list (replace toon01.bmp - toon10.bmp) */
      /* the "toon0.bmp" should be loaded separatedly */
      PMDTexture *texture = &m_localToonTexture[0];
      if (loader->loadSystemTexture(0, texture)) {
        m_toonTextureID[0] = texture->getID();
      }
      for (int i = 1; i <= kNSystemTextureFiles; i++) {
         const char *exToonBMPName = (const char *)ptr;
         texture = &m_localToonTexture[i];
         if (loader->loadModelTexture(exToonBMPName, texture)) {
           m_toonTextureID[i] = texture->getID();
         }
         ptr += 100;
      }

      /* check for remaining ptr */
      if ((unsigned long) ptr - (unsigned long) start >= size) {
         /* no rigid body / constraint ptr exist */
      } else {
         btVector3 modelOffset = (*(m_rootBone.getOffset()));
         /* update bone matrix to apply root bone offset to bone position */
         for (unsigned int i = 0; i < m_numBone; i++)
            m_boneList[i].update();

         /* Bullet Physics rigidbody ptr */
         m_numRigidBody = *((unsigned int *) ptr);
         ptr += sizeof(unsigned int);
         if (m_numRigidBody > 0) {
            m_rigidBodyList = new PMDRigidBody[m_numRigidBody];
            PMDFile_RigidBody *fileRigidBody = (PMDFile_RigidBody *) ptr;
            for (unsigned int i = 0; i < m_numRigidBody; i++) {
              PMDFile_RigidBody *rb = &fileRigidBody[i];
               if (! m_rigidBodyList[i].setup(rb, (rb->boneID == 0xFFFF) ? m_centerBone : &(m_boneList[rb->boneID])))
                  ret = false;
               m_rigidBodyList[i].joinWorld(m_bulletPhysics->getWorld());
               /* flag the bones under simulation in order to skip IK solving for those bones */
               if (rb->type != 0 && rb->boneID != 0xFFFF)
                  m_boneList[rb->boneID].setSimulatedFlag(true);
            }
            ptr += sizeof(PMDFile_RigidBody) * m_numRigidBody;
         }

         /* BulletPhysics constraint ptr */
         m_numConstraint = *((unsigned int *) ptr);
         ptr += sizeof(unsigned int);
         if (m_numConstraint > 0) {
            m_constraintList = new PMDConstraint[m_numConstraint];
            PMDFile_Constraint *fileConstraint = (PMDFile_Constraint *) ptr;
            for (unsigned int i = 0; i < m_numConstraint; i++) {
               if (!m_constraintList[i].setup(&fileConstraint[i], m_rigidBodyList, &modelOffset))
                  ret = false;
               m_constraintList[i].joinWorld(m_bulletPhysics->getWorld());
            }
            ptr += sizeof(PMDFile_Constraint) * m_numConstraint;
         }
      }
   }

   if (ret == false) goto error;

#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   /* left-handed system: PMD, DirectX */
   /* right-handed system: OpenGL, bulletphysics */
   /* convert the left-handed vertices to right-handed system */
   /* 1. vectors should be (x, y, -z) */
   /* 2. rotation should be (-rx, -ry, z) */
   /* 3. surfaces should be reversed */
   /* reverse Z value on vertices */
   for (unsigned int i = 0; i < m_numVertex; i++) {
      m_vertexList[i].setZ(-m_vertexList[i].z());
      m_normalList[i].setZ(-m_normalList[i].z());
   }
   /* reverse surface, swapping vartex order [0] and [1] in a triangle surface */
   for (unsigned int i = 0; i < m_numSurface; i += 3) {
      unsigned int j = m_surfaceList[i];
      m_surfaceList[i] = m_surfaceList[i+1];
      m_surfaceList[i+1] = j;
   }
#endif

   /* prepare work area */
   /* transforms for skinning */
   m_boneSkinningTrans = new btTransform[m_numBone];
   /* calculated Vertex informations for skinning */
   m_skinnedVertexList = new btVector3[m_numVertex];
   m_skinnedNormalList = new btVector3[m_numVertex];
   /* calculated Texture coordinates for toon shading */
   m_toonTexCoordList = static_cast<TexCoord *>(MMDAIMemoryAllocate(sizeof(TexCoord) * m_numVertex));
   if (m_toonTexCoordList == NULL)
     goto error;
   /* calculated Vertex positions for toon edge drawing */
   m_edgeVertexList = new btVector3[m_numVertex];
   /* surface list to be rendered at edge drawing (skip non-edge materials) */
   m_numSurfaceForEdge = 0;

   for (unsigned int i = 0; i < m_numMaterial; i++) {
      PMDMaterial *m = &m_material[i];
      if (m->getEdgeFlag())
         m_numSurfaceForEdge += m->getNumSurface();
   }
   if (m_numSurfaceForEdge > 0) {
      m_surfaceListForEdge = static_cast<unsigned short *>(MMDAIMemoryAllocate(sizeof(unsigned short) * m_numSurfaceForEdge));
      if (m_surfaceList == NULL)
        goto error;
      unsigned short *surfaceFrom = m_surfaceList;
      unsigned short *surfaceTo = m_surfaceListForEdge;
      for (unsigned int i = 0; i < m_numMaterial; i++) {
         PMDMaterial *m = &m_material[i];
         unsigned int n = m->getNumSurface();
         if (m->getEdgeFlag()) {
            memcpy(surfaceTo, surfaceFrom, sizeof(unsigned short) * n);
            surfaceTo += n;
         }
         surfaceFrom += n;
      }
   }

   /* check if spheremap is used (single or multiple) */
   for (unsigned int i = 0; i < m_numMaterial; i++) {
      PMDMaterial *m = &m_material[i];
      if (m->hasSingleSphereMap())
         m_hasSingleSphereMap = true;
      if (m->hasMultipleSphereMap())
         m_hasMultipleSphereMap = true;
   }

   /* make index of rotation-subjective bones (type == UNDER_ROTATE or FOLLOW_RORATE) */
   m_numRotateBone = 0;
   for (unsigned int i = 0; i < m_numBone; i++) {
      unsigned char type = m_boneList[i].getType();
      if (type == UNDER_ROTATE || type == FOLLOW_ROTATE)
         m_numRotateBone++;
   }
   if (m_numRotateBone > 0) {
      m_rotateBoneIDList = static_cast<unsigned short *>(MMDAIMemoryAllocate(sizeof(unsigned short) * m_numRotateBone));
      if (m_rotateBoneIDList == NULL)
        goto error;
      for (unsigned int i = 0, j = 0; i < m_numBone; i++) {
         unsigned char type = m_boneList[i].getType();
         if (type == UNDER_ROTATE || type == FOLLOW_ROTATE)
            m_rotateBoneIDList[j++] = i;
      }
   }

   /* check if some IK solvers can be disabled since the bones are simulated by physics */
   if (m_numIK > 0) {
      m_IKSimulated = static_cast<bool *>(MMDAIMemoryAllocate(sizeof(bool) * m_numIK));
      if (m_IKSimulated == NULL)
        goto error;
      for (unsigned int i = 0; i < m_numIK; i++) {
         /* this IK will be disabled when the leaf bone is controlled by physics simulation */
         m_IKSimulated[i] = m_IKList[i].isSimulated();
      }
   }

   /* mark motion independency for each bone */
   for (unsigned int i = 0; i < m_numBone; i++)
      m_boneList[i].setMotionIndependency();

   /* build name->entity index for fast lookup */
   for (unsigned int i = 0; i < m_numBone; i++) {
      PMDBone *bone = &m_boneList[i];
      const char *name = bone->getName();
      PMDBone *bMatch = (PMDBone *) m_name2bone.findNearest(name);
      if (bMatch == NULL || !MMDAIStringEquals(bMatch->getName(), name))
         m_name2bone.add(name, bone, (bMatch) ? bMatch->getName() : NULL); /* add */
   }
   for (unsigned int i = 0; i < m_numFace; i++) {
      PMDFace *face = &m_faceList[i];
      const char *name = face->getName();
      PMDFace *fMatch = (PMDFace *) m_name2face.findNearest(name);
      if (fMatch == NULL || !MMDAIStringEquals(fMatch->getName(), name))
         m_name2face.add(name, face, (fMatch) ? fMatch->getName() : NULL); /* add */
   }

   /* get maximum height */
   if (m_numVertex > 0) {
      m_maxHeight = m_vertexList[0].y();
      for (unsigned int i = 1; i < m_numVertex; i++) {
         float y = m_vertexList[i].y();
         if (m_maxHeight < y)
            m_maxHeight = y;
      }
   }

   /* simulation is currently off, so change bone status */
   if (!m_enableSimulation)
      setPhysicsControl(false);
   loader->unloadModelData(start);

   return true;

error:
   loader->unloadModelData(start);
   release();
   return false;
}

