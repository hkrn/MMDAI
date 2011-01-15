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

#include "MMDFiles.h"

/* PMDModel::parse: initialize and load from data memories */
bool PMDModel::parse(unsigned char *data, unsigned long size, BulletPhysics *bullet, SystemTexture *systex, char *dir)
{
   unsigned char *start = data;
   FILE *fp;
   bool ret = true;
   unsigned long i;
   btQuaternion defaultRot;

   PMDFile_Header *fileHeader;
   PMDFile_Vertex *fileVertex;
   PMDFile_Material *fileMaterial;
   PMDFile_Bone *fileBone;
   PMDFile_IK *fileIK;
   PMDFile_Face *fileFace;

   unsigned char numFaceDisp;
   unsigned char numBoneFrameDisp;
   unsigned long numBoneDisp;

   char buf[MMDFILES_MAXBUFLEN]; /* for toon texture */
   const char centerBoneName[] = MOTIONCONTROLLER_CENTERBONENAME;

   unsigned char englishNameExist;
   char *exToonBMPName;

   btVector3 modelOffset;
   PMDFile_RigidBody *fileRigidBody;
   PMDFile_Constraint *fileConstraint;

   unsigned short j, k;
   unsigned short *surfaceFrom, *surfaceTo;
   unsigned char type;
   char *name;
   PMDBone *bMatch;
   PMDFace *fMatch;

   /* clear memory */
   clear();
   m_hasSingleSphereMap = false;
   m_hasMultipleSphereMap = false;
   m_baseFace = NULL;
   m_centerBone = NULL;

   /* reset root bone's rotation */
   defaultRot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
   m_rootBone.setCurrentRotation(&defaultRot);
   m_rootBone.update();

   /* set Bullet Physics */
   m_bulletPhysics = bullet;

   /* reset toon texture IDs by system default textures */
   for (j = 0; j < SYSTEMTEXTURE_NUMFILES; j++)
      m_toonTextureID[j] = systex->getTextureID(j);

   /* header */
   fileHeader = (PMDFile_Header *) data;
   if (fileHeader->magic[0] != 'P' || fileHeader->magic[1] != 'm' || fileHeader->magic[2] != 'd')
      return false;
   if (fileHeader->version != 1.0f)
      return false;
   /* name */
   m_name = (char *) malloc(sizeof(char) * (20 + 1));
   strncpy(m_name, fileHeader->name, 20);
   m_name[20] = '\0';

   /* directory */
   m_modelDir = strdup(dir);
   /* comment */
   m_comment = (char *) malloc(sizeof(char) * (256 + 1));
   strncpy(m_comment, fileHeader->comment, 256);
   m_comment[256] = '\0';
   data += sizeof(PMDFile_Header);

   /* vertex data and bone weights */
   /* relocate as separated list for later OpenGL calls */
   // FIXME: unsigned long is 64bit under 64bit environment
   m_numVertex = *((unsigned int *) data);
   data += sizeof(unsigned int);
   m_vertexList = new btVector3[m_numVertex];
   m_normalList = new btVector3[m_numVertex];
   m_texCoordList = (TexCoord *) malloc(sizeof(TexCoord) * m_numVertex);
   m_bone1List = (short *) malloc(sizeof(short) * m_numVertex);
   m_bone2List = (short *) malloc(sizeof(short) * m_numVertex);
   m_boneWeight1 = (float *) malloc(sizeof(float) * m_numVertex);
   m_noEdgeFlag = (bool *) malloc(sizeof(bool) * m_numVertex);
   fileVertex = (PMDFile_Vertex *) data;
   for (i = 0; i < m_numVertex; i++) {
      m_vertexList[i].setValue(fileVertex[i].pos[0], fileVertex[i].pos[1], fileVertex[i].pos[2]);
      m_normalList[i].setValue(fileVertex[i].normal[0], fileVertex[i].normal[1], fileVertex[i].normal[2]);
      m_texCoordList[i].u = fileVertex[i].uv[0];
      m_texCoordList[i].v = fileVertex[i].uv[1];
      m_bone1List[i] = fileVertex[i].boneID[0];
      m_bone2List[i] = fileVertex[i].boneID[1];
      m_boneWeight1[i] = fileVertex[i].boneWeight1 * 0.01f;
      m_noEdgeFlag[i] = fileVertex[i].noEdgeFlag ? true : false;
   }
   data += sizeof(PMDFile_Vertex) * m_numVertex;

   /* surface data, 3 vertex indices for each */
   m_numSurface = *((unsigned int *) data);
   data += sizeof(unsigned int);
   m_surfaceList = (unsigned short *) malloc(sizeof(unsigned short) * m_numSurface);
   memcpy(m_surfaceList, data, sizeof(unsigned short) * m_numSurface);
   data += sizeof(unsigned short) * m_numSurface;

   /* material data (color, texture, toon parameter, edge flag) */
   m_numMaterial = *((unsigned int *) data);
   data += sizeof(unsigned int);
   m_material = new PMDMaterial[m_numMaterial];
   fileMaterial = (PMDFile_Material *) data;
   for (i = 0; i < m_numMaterial; i++) {
      if (!m_material[i].setup(&fileMaterial[i], &m_textureLoader, dir)) {
         /* ret = false; */
      }
   }
   data += sizeof(PMDFile_Material) * m_numMaterial;

   /* bone data */
   m_numBone = *((unsigned short *) data);
   data += sizeof(unsigned short);
   m_boneList = new PMDBone[m_numBone];
   fileBone = (PMDFile_Bone *) data;
   for (i = 0; i < m_numBone; i++) {
      if (!m_boneList[i].setup(&(fileBone[i]), m_boneList, m_numBone, &m_rootBone))
         ret = false;
      if (strcmp(m_boneList[i].getName(), centerBoneName) == 0)
         m_centerBone = &(m_boneList[i]);
   }
   if (!m_centerBone && m_numBone >= 1) {
      /* if no bone is named "center," use the first bone as center */
      m_centerBone = &(m_boneList[0]);
   }
   data += sizeof(PMDFile_Bone) * m_numBone;
   /* calculate bone offset after all bone positions are loaded */
   for (i = 0; i < m_numBone; i++)
      m_boneList[i].computeOffset();

   /* IK data */
   m_numIK = *((unsigned short *) data);
   data += sizeof(unsigned short);
   if (m_numIK > 0) {
      m_IKList = new PMDIK[m_numIK];
      for (i = 0; i < m_numIK; i++) {
         fileIK = (PMDFile_IK *) data;
         data += sizeof(PMDFile_IK);
         m_IKList[i].setup(fileIK, (short *)data, m_boneList);
         data += sizeof(short) * fileIK->numLink;
      }
   }

   /* face data */
   m_numFace = *((unsigned short *) data);
   data += sizeof(unsigned short);
   if (m_numFace > 0) {
      m_faceList = new PMDFace[m_numFace];
      for (i = 0; i < m_numFace; i++) {
         fileFace = (PMDFile_Face *)data;
         data += sizeof(PMDFile_Face);
         m_faceList[i].setup(fileFace, (PMDFile_Face_Vertex *) data);
         if (fileFace->type == PMD_FACE_BASE)
            m_baseFace = &(m_faceList[i]); /* store base face */
         data += sizeof(PMDFile_Face_Vertex) * fileFace->numVertex;
      }
      if (m_baseFace == NULL) {
         ret = false;
      } else {
         /* convert base-relative index to the index of original vertices */
         for (i = 0; i < m_numFace; i++)
            m_faceList[i].convertIndex(m_baseFace);
      }
   }

   /* display names (skip) */
   /* indices for faces which should be displayed in "face" region */
   numFaceDisp = *((unsigned char *) data);
   data += sizeof(unsigned char) + sizeof(unsigned short) * numFaceDisp;
   /* bone frame names */
   numBoneFrameDisp = *((unsigned char *) data);
   data += sizeof(unsigned char) + 50 * numBoneFrameDisp;
   /* indices for bones which should be displayed in each bone region */
   numBoneDisp = *((unsigned int *) data);
   data += sizeof(unsigned int) + (sizeof(short) + sizeof(unsigned char)) * numBoneDisp;

   /* end of base format */
   /* check for remaining data */
   if ((unsigned long) data - (unsigned long) start >= size) {
      /* no extension data remains */
      m_numRigidBody = 0;
      m_numConstraint = 0;
      /* assign default toon textures for toon shading */
      for (i = 0; i <= 10; i++) {
         if (i == 0)
            sprintf(buf, "%s%ctoon0.bmp", dir, MMDFILES_DIRSEPARATOR);
         else
            sprintf(buf, "%s%ctoon%02d.bmp", dir, MMDFILES_DIRSEPARATOR, i);
         /* if "toon??.bmp" exist at the same directory as PMD file, use it */
         /* if not exist or failed to read, use system default toon textures */
         fp = fopen(buf, "rb");
         if (fp != NULL) {
            fclose(fp);
            if (m_localToonTexture[i].load(buf) == true)
               m_toonTextureID[i] = m_localToonTexture[i].getID();
         }
      }
   } else {
      /* English display names (skip) */
      englishNameExist = *((unsigned char *) data);
      data += sizeof(unsigned char);
      if (englishNameExist != 0) {
         /* model name and comments in English */
         data += 20 + 256;
         /* bone names in English */
         data += 20 * m_numBone;
         /* face names in English */
         if (m_numFace > 0) data += 20 * (m_numFace - 1); /* "base" not included in English list */
         /* bone frame names in English */
         data += 50 * numBoneFrameDisp;
      }

      /* toon texture file list (replace toon01.bmp - toon10.bmp) */
      /* the "toon0.bmp" should be loaded separatedly */
      sprintf(buf, "%s%ctoon0.bmp", dir, MMDFILES_DIRSEPARATOR);
      fp = fopen(buf, "rb");
      if (fp != NULL) {
         fclose(fp);
         if (m_localToonTexture[0].load(buf) == true)
            m_toonTextureID[0] = m_localToonTexture[0].getID();
      }
      for (i = 1; i <= 10; i++) {
         exToonBMPName = (char *) data;
         sprintf(buf, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, exToonBMPName);
         fp = fopen(buf, "rb");
         if (fp != NULL) {
            fclose(fp);
            if (m_localToonTexture[i].load(buf) == true)
               m_toonTextureID[i] = m_localToonTexture[i].getID();
         }
         data += 100;
      }

      /* check for remaining data */
      if ((unsigned long) data - (unsigned long) start >= size) {
         /* no rigid body / constraint data exist */
      } else if (!m_bulletPhysics) {
         /* check if we have given a bulletphysics engine */
      } else {
         modelOffset = (*(m_rootBone.getOffset()));
         /* update bone matrix to apply root bone offset to bone position */
         for (i = 0; i < m_numBone; i++)
            m_boneList[i].update();

         /* Bullet Physics rigidbody data */
         m_numRigidBody = *((unsigned int *) data);
         data += sizeof(unsigned int);
         if (m_numRigidBody > 0) {
            m_rigidBodyList = new PMDRigidBody[m_numRigidBody];
            fileRigidBody = (PMDFile_RigidBody *) data;
            for (i = 0; i < m_numRigidBody; i++) {
               if (! m_rigidBodyList[i].setup(&fileRigidBody[i], (fileRigidBody[i].boneID == 0xFFFF) ? m_centerBone : &(m_boneList[fileRigidBody[i].boneID])))
                  ret = false;
               m_rigidBodyList[i].joinWorld(m_bulletPhysics->getWorld());
               /* flag the bones under simulation in order to skip IK solving for those bones */
               if (fileRigidBody[i].type != 0 && fileRigidBody[i].boneID != 0xFFFF)
                  m_boneList[fileRigidBody[i].boneID].setSimulatedFlag(true);
            }
            data += sizeof(PMDFile_RigidBody) * m_numRigidBody;
         }

         /* BulletPhysics constraint data */
         m_numConstraint = *((unsigned int *) data);
         data += sizeof(unsigned int);
         if (m_numConstraint > 0) {
            m_constraintList = new PMDConstraint[m_numConstraint];
            fileConstraint = (PMDFile_Constraint *) data;
            for (i = 0; i < m_numConstraint; i++) {
               if (!m_constraintList[i].setup(&fileConstraint[i], m_rigidBodyList, &modelOffset))
                  ret = false;
               m_constraintList[i].joinWorld(m_bulletPhysics->getWorld());
            }
            data += sizeof(PMDFile_Constraint) * m_numConstraint;
         }
      }
   }

   if (ret == false) return false;

#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   /* left-handed system: PMD, DirectX */
   /* right-handed system: OpenGL, bulletphysics */
   /* convert the left-handed vertices to right-handed system */
   /* 1. vectors should be (x, y, -z) */
   /* 2. rotation should be (-rx, -ry, z) */
   /* 3. surfaces should be reversed */
   /* reverse Z value on vertices */
   for (i = 0; i < m_numVertex; i++) {
      m_vertexList[i].setZ(-m_vertexList[i].z());
      m_normalList[i].setZ(-m_normalList[i].z());
   }
   /* reverse surface, swapping vartex order [0] and [1] in a triangle surface */
   for (i = 0; i < m_numSurface; i += 3) {
      j = m_surfaceList[i];
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
   m_toonTexCoordList = (TexCoord *) malloc(sizeof(TexCoord) * m_numVertex);
   /* calculated Vertex positions for toon edge drawing */
   m_edgeVertexList = new btVector3[m_numVertex];
   /* surface list to be rendered at edge drawing (skip non-edge materials) */
   m_numSurfaceForEdge = 0;
   for (i = 0; i < m_numMaterial; i++)
      if (m_material[i].getEdgeFlag())
         m_numSurfaceForEdge += m_material[i].getNumSurface();
   if (m_numSurfaceForEdge > 0) {
      m_surfaceListForEdge = (unsigned short *) malloc(sizeof(unsigned short) * m_numSurfaceForEdge);
      surfaceFrom = m_surfaceList;
      surfaceTo = m_surfaceListForEdge;
      for (i = 0; i < m_numMaterial; i++) {
         if (m_material[i].getEdgeFlag()) {
            memcpy(surfaceTo, surfaceFrom, sizeof(unsigned short) * m_material[i].getNumSurface());
            surfaceTo += m_material[i].getNumSurface();
         }
         surfaceFrom += m_material[i].getNumSurface();
      }
   }
   /* check if spheremap is used (single or multiple) */
   for (i = 0; i < m_numMaterial; i++) {
      if (m_material[i].hasSingleSphereMap())
         m_hasSingleSphereMap = true;
      if (m_material[i].hasMultipleSphereMap())
         m_hasMultipleSphereMap = true;
   }

   /* make index of rotation-subjective bones (type == UNDER_ROTATE or FOLLOW_RORATE) */
   m_numRotateBone = 0;
   for (j = 0; j < m_numBone; j++) {
      type = m_boneList[j].getType();
      if (type == UNDER_ROTATE || type == FOLLOW_ROTATE)
         m_numRotateBone++;
   }
   if (m_numRotateBone > 0) {
      m_rotateBoneIDList = (unsigned short *) malloc(sizeof(unsigned short) * m_numRotateBone);
      for (j = 0, k = 0; j < m_numBone; j++) {
         type = m_boneList[j].getType();
         if (type == UNDER_ROTATE || type == FOLLOW_ROTATE)
            m_rotateBoneIDList[k++] = j;
      }
   }

   /* check if some IK solvers can be disabled since the bones are simulated by physics */
   if (m_numIK > 0) {
      m_IKSimulated = (bool *) malloc(sizeof(bool) * m_numIK);
      for (j = 0; j < m_numIK; j++) {
         /* this IK will be disabled when the leaf bone is controlled by physics simulation */
         m_IKSimulated[j] = m_IKList[j].isSimulated();
      }
   }

   /* mark motion independency for each bone */
   for (j = 0; j < m_numBone; j++)
      m_boneList[j].setMotionIndependency();

   /* build name->entity index for fast lookup */
   for (j = 0; j < m_numBone; j++) {
      name = m_boneList[j].getName();
      bMatch = (PMDBone *) m_name2bone.findNearest(name);
      if (bMatch == NULL || strcmp(bMatch->getName(), name) != 0)
         m_name2bone.add(name, &(m_boneList[j]), (bMatch) ? bMatch->getName() : NULL); /* add */
   }
   for (j = 0; j < m_numFace; j++) {
      name = m_faceList[j].getName();
      fMatch = (PMDFace *) m_name2face.findNearest(name);
      if (fMatch == NULL || strcmp(fMatch->getName(), name) != 0)
         m_name2face.add(name, &(m_faceList[j]), (fMatch) ? fMatch->getName() : NULL); /* add */
   }

   /* get maximum height */
   if (m_numVertex > 0) {
      m_maxHeight = m_vertexList[0].y();
      for (i = 1; i < m_numVertex; i++)
         if (m_maxHeight < m_vertexList[i].y())
            m_maxHeight = m_vertexList[i].y();
   }

   /* simulation is currently off, so change bone status */
   if (!m_enableSimulation)
      setPhysicsControl(false);

   return true;
}
