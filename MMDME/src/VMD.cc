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

/* compareKeyFrameBone: qsort function for bone key frames */
static int compareKeyFrameBone(const void *x, const void *y)
{
   BoneKeyFrame *a = (BoneKeyFrame *) x;
   BoneKeyFrame *b = (BoneKeyFrame *) y;

   return (int) (a->keyFrame - b->keyFrame);
}

/* compareKeyFrameFace: qsort function for face key frames */
static int compareKeyFrameFace(const void *x, const void *y)
{
   FaceKeyFrame *a = (FaceKeyFrame *) x;
   FaceKeyFrame *b = (FaceKeyFrame *) y;

   return (int) (a->keyFrame - b->keyFrame);
}

/* ipfunc: t->value for 4-point (3-dim.) bezier curve */
static float ipfunc(float t, float p1, float p2)
{
   return ((1 + 3 * p1 - 3 * p2) * t * t * t + (3 * p2 - 6 * p1) * t * t + 3 * p1 * t);
}

/* ipfuncd: derivation of ipfunc */
static float ipfuncd(float t, float p1, float p2)
{
   return ((3 + 9 * p1 - 9 * p2) * t * t + (6 * p2 - 12 * p1) * t + 3 * p1);
}

/* VMD::addBoneMotion: add new bone motion to list */
void VMD::addBoneMotion(const char *name)
{
   BoneMotionLink *link;
   BoneMotion *bmNew;
   BoneMotion *bmMatch;

   if(name == NULL) return;

   link = static_cast<BoneMotionLink *>(MMDAIMemoryAllocate(sizeof(BoneMotionLink)));
   if (link == NULL)
      return;
   bmNew = &(link->boneMotion);
   bmNew->name = MMDAIStringClone(name);
   bmNew->numKeyFrame = 1;
   bmNew->keyFrameList = NULL;

   link->next = m_boneLink;
   m_boneLink = link;

   bmMatch = (BoneMotion *) m_name2bone.findNearest(name);
   if (!bmMatch || !MMDAIStringEquals(bmMatch->name, name))
      m_name2bone.add(name, bmNew, (bmMatch) ? bmMatch->name : NULL);
}

/* VMD::addFaceMotion: add new face motion to list */
void VMD::addFaceMotion(const char *name)
{
   FaceMotionLink *link;
   FaceMotion *fmNew;
   FaceMotion *fmMatch;

   if(name == NULL) return;

   link = static_cast<FaceMotionLink *>(MMDAIMemoryAllocate(sizeof(FaceMotionLink)));
   if (link == NULL)
      return;
   fmNew = &(link->faceMotion);
   fmNew->name = MMDAIStringClone(name);
   fmNew->numKeyFrame = 1;
   fmNew->keyFrameList = NULL;

   link->next = m_faceLink;
   m_faceLink = link;

   fmMatch = (FaceMotion *) m_name2face.findNearest(name);
   if (!fmMatch || !MMDAIStringEquals(fmMatch->name, name))
      m_name2face.add(name, fmNew, (fmMatch) ? fmMatch->name : NULL);
}

/* VMD::getBoneMotion: find bone motion by name */
BoneMotion* VMD::getBoneMotion(const char *name)
{
   BoneMotion *bm;

   if (name == NULL)
      return NULL;

   bm = (BoneMotion *) m_name2bone.findNearest(name);
   if (bm && MMDAIStringEquals(bm->name, name))
      return bm;

   return NULL;
}

/* VMD::getFaceMotion: find face motion by name */
FaceMotion* VMD::getFaceMotion(const char *name)
{
   FaceMotion *fm;

   if(name == NULL)
      return NULL;

   fm = (FaceMotion *) m_name2face.findNearest(name);
   if (fm && MMDAIStringEquals(fm->name, name))
      return fm;

   return NULL;
}

/* VMD::setInterpolationTable: set up motion interpolation parameter */
void VMD::setInterpolationTable(BoneKeyFrame *bf, char ip[])
{
   short i, d;
   float x1, x2, y1, y2;
   float inval, t, v, tt;

   /* check if they are just a linear function */
   for (i = 0; i < 4; i++)
      bf->linear[i] = (ip[0+i] == ip[4+i] && ip[8+i] == ip[12+i]) ? true : false;

   /* make X (0.0 - 1.0) -> Y (0.0 - 1.0) mapping table */
   for (i = 0; i < 4; i++) {
      if (bf->linear[i]) {
         /* table not needed */
         bf->interpolationTable[i] = NULL;
         continue;
      }
      bf->interpolationTable[i] = static_cast<float *>(MMDAIMemoryAllocate(sizeof(float) * kInterpolationTableSize));
      if (bf->interpolationTable[i] == NULL)
         return;
      x1 = ip[   i] / 127.0f;
      y1 = ip[ 4+i] / 127.0f;
      x2 = ip[ 8+i] / 127.0f;
      y2 = ip[12+i] / 127.0f;
      for (d = 0; d < kInterpolationTableSize; d++) {
         inval = ((float) d + 0.5f) / (float) kInterpolationTableSize;
         /* get Y value for given inval */
         t = inval;
         while (1) {
            v = ipfunc(t, x1, x2) - inval;
            if (fabsf(v) < 0.0001f) break;
            tt = ipfuncd(t, x1, x2);
            if (tt == 0.0f) break;
            t -= v / tt;
         }
         bf->interpolationTable[i][d] = ipfunc(t, y1, y2);
      }
   }
}

/* VMD::initialize: initialize VMD */
void VMD::initialize()
{
   m_numTotalBoneKeyFrame = 0;
   m_numTotalFaceKeyFrame = 0;
   m_boneLink = NULL;
   m_faceLink = NULL;
   m_numBoneKind = 0;
   m_numBoneKind = 0;
   m_maxFrame = 0.0f;
}

/* VMD::clear: free VMD */
void VMD::clear()
{
   BoneMotionLink *bl, *bl_tmp;
   FaceMotionLink *fl, *fl_tmp;
   unsigned long i;
   short j;

   m_name2bone.release();
   m_name2face.release();

   bl = m_boneLink;
   while (bl) {
      if (bl->boneMotion.keyFrameList) {
         for (i = 0; i < bl->boneMotion.numKeyFrame; i++)
            for (j = 0; j < 4; j++)
               if (bl->boneMotion.keyFrameList[i].linear[j] == false)
                  MMDAIMemoryRelease(bl->boneMotion.keyFrameList[i].interpolationTable[j]);
         MMDAIMemoryRelease(bl->boneMotion.keyFrameList);
      }
      if(bl->boneMotion.name)
         MMDAIMemoryRelease(bl->boneMotion.name);
      bl_tmp = bl->next;
      MMDAIMemoryRelease(bl);
      bl = bl_tmp;
   }

   fl = m_faceLink;
   while (fl) {
      if (fl->faceMotion.keyFrameList)
         MMDAIMemoryRelease(fl->faceMotion.keyFrameList);
      if(fl->faceMotion.name)
         MMDAIMemoryRelease(fl->faceMotion.name);
      fl_tmp = fl->next;
      MMDAIMemoryRelease(fl);
      fl = fl_tmp;
   }

   initialize();
}

/* VMD::VMD: constructor */
VMD::VMD()
{
   initialize();
}

/* VMD::~VMD: destructor */
VMD::~VMD()
{
   clear();
}

/* VMD::load: initialize and load from file name */
bool VMD::load(VMDLoader *loader)
{
   unsigned char *ptr = NULL;
   size_t size = 0;
   if (!loader->loadMotionData(&ptr, &size))
     return false;
   bool ret = parse(ptr, size);
   loader->unloadMotionData(ptr);
   return ret;
}

/* VMD::parse: initialize and load from data memories */
bool VMD::parse(unsigned char *data, size_t size)
{
   unsigned long i;
   BoneMotion *bm;
   BoneMotionLink *bl;
   FaceMotion *fm;
   FaceMotionLink *fl;

   VMDFile_Header *header;
   VMDFile_BoneFrame *boneFrame;
   VMDFile_FaceFrame *faceFrame;

   char name[16];
   (void)size;

   /* free VMD */
   clear();

   /* header */
   header = (VMDFile_Header *) data;
   if (!MMDAIStringEqualsIn(header->header, "Vocaloid Motion Data 0002", sizeof(header->header)))
      return false;

   data += sizeof(VMDFile_Header);

   /* bone motions */
   // FIXME: use unsigned int instead of unsigned long for 64bit environment
   m_numTotalBoneKeyFrame = *((unsigned int *) data);
   data += sizeof(unsigned int);

   boneFrame = (VMDFile_BoneFrame *) data;

   /* list bones that exists in the data and count the number of defined key frames for each */
   for (i = 0; i < m_numTotalBoneKeyFrame; i++) {
      MMDAIStringCopy(name, boneFrame[i].name, 15);
      name[15] = '\0';
      bm = getBoneMotion(name);
      if (bm)
         bm->numKeyFrame++;
      else
         addBoneMotion(name);
   }
   /* allocate memory to store the key frames, and reset count again */
   for (bl = m_boneLink; bl; bl = bl->next) {
      bl->boneMotion.keyFrameList = static_cast<BoneKeyFrame *>(MMDAIMemoryAllocate(sizeof(BoneKeyFrame) * bl->boneMotion.numKeyFrame));
      bl->boneMotion.numKeyFrame = 0;
   }
   /* store the key frames, parsing the data again. also compute max frame */
   for (i = 0; i < m_numTotalBoneKeyFrame; i++) {
      MMDAIStringCopy(name, boneFrame[i].name, 15);
      name[15] = '\0';
      bm = getBoneMotion(name);
      if (bm) {
         bm->keyFrameList[bm->numKeyFrame].keyFrame = (float) boneFrame[i].keyFrame;
         if (m_maxFrame < bm->keyFrameList[bm->numKeyFrame].keyFrame)
            m_maxFrame = bm->keyFrameList[bm->numKeyFrame].keyFrame;
         /* convert from left-hand coordinates to right-hand coordinates */
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
         bm->keyFrameList[bm->numKeyFrame].pos = btVector3(boneFrame[i].pos[0], boneFrame[i].pos[1], -boneFrame[i].pos[2]);
         bm->keyFrameList[bm->numKeyFrame].rot = btQuaternion(-boneFrame[i].rot[0], -boneFrame[i].rot[1], boneFrame[i].rot[2], boneFrame[i].rot[3]);
#else
         bm->keyFrameList[bm->numKeyFrame].pos = btVector3(boneFrame[i].pos[0], boneFrame[i].pos[1], boneFrame[i].pos[2]);
         bm->keyFrameList[bm->numKeyFrame].rot = btQuaternion(boneFrame[i].rot[0], boneFrame[i].rot[1], boneFrame[i].rot[2], boneFrame[i].rot[3]);
#endif
         /* set interpolation table */
         setInterpolationTable(&(bm->keyFrameList[bm->numKeyFrame]), boneFrame[i].interpolation);
         bm->numKeyFrame++;
      }
   }
   /* sort the key frames in each boneMotion by frame */
   for (bl = m_boneLink; bl; bl = bl->next)
      qsort(bl->boneMotion.keyFrameList, bl->boneMotion.numKeyFrame, sizeof(BoneKeyFrame), compareKeyFrameBone);
   /* count number of bones appear in this vmd */
   m_numBoneKind = 0;
   for (bl = m_boneLink; bl; bl = bl->next)
      m_numBoneKind++;

   data += sizeof(VMDFile_BoneFrame) * m_numTotalBoneKeyFrame;

   /* face motions */
   m_numTotalFaceKeyFrame = *((unsigned int *) data);
   data += sizeof(unsigned int);

   faceFrame = (VMDFile_FaceFrame *) data;

   /* list faces that exists in the data and count the number of defined key frames for each */
   for (i = 0; i < m_numTotalFaceKeyFrame; i++) {
      MMDAIStringCopy(name, faceFrame[i].name, 15);
      name[15] = '\0';
      fm = getFaceMotion(name);
      if (fm)
         fm->numKeyFrame++;
      else
         addFaceMotion(name);
   }
   /* allocate memory to store the key frames, and reset count again */
   for (fl = m_faceLink; fl; fl = fl->next) {
      fl->faceMotion.keyFrameList = static_cast<FaceKeyFrame *>(MMDAIMemoryAllocate(sizeof(FaceKeyFrame) * fl->faceMotion.numKeyFrame));
      fl->faceMotion.numKeyFrame = 0;
   }
   /* store the key frames, parsing the data again. also compute max frame */
   for (i = 0; i < m_numTotalFaceKeyFrame; i++) {
      MMDAIStringCopy(name, faceFrame[i].name, 15);
      name[15] = '\0';
      fm = getFaceMotion(faceFrame[i].name);
      if (fm) {
         fm->keyFrameList[fm->numKeyFrame].keyFrame = (float) faceFrame[i].keyFrame;
         if (m_maxFrame < fm->keyFrameList[fm->numKeyFrame].keyFrame)
            m_maxFrame = fm->keyFrameList[fm->numKeyFrame].keyFrame;
         fm->keyFrameList[fm->numKeyFrame].weight = faceFrame[i].weight;
         fm->numKeyFrame++;
      }
   }
   /* sort the key frames in each faceMotion by frame */
   for (fl = m_faceLink; fl; fl = fl->next)
      qsort(fl->faceMotion.keyFrameList, fl->faceMotion.numKeyFrame, sizeof(FaceKeyFrame), compareKeyFrameFace);

   /* count number of faces appear in this vmd */
   m_numFaceKind = 0;
   for (fl = m_faceLink; fl; fl = fl->next)
      m_numFaceKind++;

   data += sizeof(VMDFile_FaceFrame) * m_numTotalFaceKeyFrame;

   return true;
}

/* VMD::getTotalKeyFrame: get total number of key frames */
unsigned long VMD::getTotalKeyFrame() const
{
   return m_numTotalBoneKeyFrame + m_numTotalFaceKeyFrame;
}

/* VMD::getBoneMotionLink: get list of bone motions */
BoneMotionLink *VMD::getBoneMotionLink() const
{
   return m_boneLink;
}

/* VMD::getFaceMotionLink: get list of face motions */
FaceMotionLink *VMD::getFaceMotionLink() const
{
   return m_faceLink;
}

/* VMD::getNumBoneKind: get number of bone motions */
unsigned int VMD::getNumBoneKind() const
{
   return m_numBoneKind;
}

/* VMD::getNumFaceKind: get number of face motions */
unsigned int VMD::getNumFaceKind() const
{
   return m_numFaceKind;
}

/* VMD::getMaxFrame: get max frame */
float VMD::getMaxFrame() const
{
   return m_maxFrame;
}

} /* namespace */

