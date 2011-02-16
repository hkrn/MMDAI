/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
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

/* headers */

#include "MMDME/VMDFile.h"
#include "MMDAI/LipSync.h"
#include "MMDAI/LipSyncLoader.h"

const float LipSync::kInterpolationRate = 0.8f;

/* LipSync::initialize: initialize lipsync */
void LipSync::initialize()
{
   m_numMotion = 0;
   m_motion = NULL;

   m_numPhone = 0;
   m_phone = NULL;
   m_blendRate = NULL;
}

/* LipSync::clear: free lipsync */
void LipSync::clear()
{
   int i;

   if(m_motion != NULL) {
      for(i = 0; i < m_numMotion; i++)
         MMDAIMemoryRelease(m_motion[i]);
      MMDAIMemoryRelease(m_motion);
   }
   if(m_phone != NULL) {
      for(i = 0; i < m_numPhone; i++)
         MMDAIMemoryRelease(m_phone[i]);
      MMDAIMemoryRelease(m_phone);
   }
   if(m_blendRate != NULL) {
      for(i = 0; i < m_numPhone; i++)
         MMDAIMemoryRelease(m_blendRate[i]);
      MMDAIMemoryRelease(m_blendRate);
   }

   initialize();
}

/* LipSync::LipSync: constructor */
LipSync::LipSync()
{
   initialize();
}

/* LipSync::~LipSync: destructor */
LipSync::~LipSync()
{
   clear();
}

/* LipSync::load: initialize and load lip setting */
bool LipSync::load(LipSyncLoader *loader)
{
   int len = 0;

   if (!loader->load())
     return false;

   m_numMotion = len = loader->getNExpressions();
   m_motion = static_cast<char **>(MMDAIMemoryAllocate(sizeof(char *) * m_numMotion));
   if (m_motion == NULL) {
     clear();
     return false;
   }
   for (int i = 0; i < len; i++) {
     const char *name = loader->getExpressionName(i);
     char *s = NULL;
     if (name == NULL || (s = MMDAIStringClone(name)) == NULL) {
       clear();
       return false;
     }
     m_motion[i] = s;
   }

   m_numPhone = len = loader->getNPhonemes();
   m_phone = static_cast<char **>(MMDAIMemoryAllocate(sizeof(char *) * m_numPhone));
   m_blendRate = static_cast<float **>(MMDAIMemoryAllocate(sizeof(float *) * m_numPhone));
   if (m_phone == NULL || m_blendRate == NULL) {
     clear();
     return false;
   }
   for (int i = 0; i < len; i++) {
     const char *name = loader->getPhoneName(i);
     char *s = NULL;
     if (name == NULL || (s = MMDAIStringClone(name)) == NULL) {
       clear();
       return false;
     }
     m_phone[i] = s;
     m_blendRate[i] = static_cast<float *>(MMDAIMemoryAllocate(sizeof(float) * m_numMotion));
     if (m_blendRate[i] == NULL) {
       clear();
       return false;
     }
     for (int j = 0; j < m_numMotion; j++) {
       float f = loader->getInterpolationWeight(i, j);
       m_blendRate[i][j] = f;
     }
   }

   return true;
}

/* LipSync::createMotion: create motion from phoneme sequence */
bool LipSync::createMotion(const char *str, unsigned char **rawData, size_t *rawSize)
{
   int i, j, k;
   int len;
   char *buf, *p, *save;
   bool ret = false;

   LipKeyFrame *head, *tail, *tmp1, *tmp2;
   float f, diff;

   int totalNumKey;
   unsigned int currentFrame;
   unsigned char *data;
   VMDFile_Header *header;
   unsigned int *numBoneKeyFrames;
   unsigned int *numFaceKeyFrames;
   VMDFile_FaceFrame *face;

   /* check */
   if(str == NULL || m_numMotion <= 0 || m_numPhone <= 0)
      return ret;

   /* initialize */
   (*rawData) = NULL;
   (*rawSize) = 0;

   /* get phone index and duration */
   buf = MMDAIStringClone(str);
   head = NULL;
   tail = NULL;
   diff = 0.0f;
#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32)
   for(i = 0, k = 0, p = strtok_r(buf, ",", &save); p; i++, p = strtok_r(NULL, ",", &save)) {
#else
   (void)save;
   for(i = 0, k = 0, p = strtok(buf, ","); p; i++, p = strtok(NULL, ",")) {
#endif
      if(i % 2 == 0) {
         for(j = 0; j < m_numPhone; j++) {
            if (MMDAIStringEquals(m_phone[j], p)) {
               k = j;
               break;
            }
         }
         if(m_numPhone <= j)
            k = 0;
      } else {
         tmp1 = static_cast<LipKeyFrame *>(MMDAIMemoryAllocate(sizeof(LipKeyFrame)));
         if (tmp1 == NULL)
           goto finally;
         tmp1->phone = k;
         f = 0.03f * atof(p) + diff; /* convert ms to frame */
         tmp1->duration = (int) (f + 0.5);
         if(tmp1->duration < 1)
            tmp1->duration = 1;
         diff = f - tmp1->duration;
         tmp1->rate = 1.0f;
         tmp1->next = NULL;
         if(head == NULL)
            head = tmp1;
         else
            tail->next = tmp1;
         tail = tmp1;
      }
   }

   /* add final closed lip */
   tmp1 = static_cast<LipKeyFrame *>(MMDAIMemoryAllocate(sizeof(LipKeyFrame)));
   if (tmp1 == NULL)
     goto finally;
   tmp1->phone = 0;
   tmp1->duration = 1;
   tmp1->rate = 0.0f;
   tmp1->next = NULL;
   if(head == NULL)
      head = tmp1;
   else
      tail->next = tmp1;
   tail = tmp1;

   /* insert interpolation lip motion */
   for(tmp1 = head; tmp1; tmp1 = tmp1->next) {
      if(tmp1->next && tmp1->duration > kInterpolationMargin) {
         tmp2 = static_cast<LipKeyFrame *>(MMDAIMemoryAllocate(sizeof(LipKeyFrame)));
         if (tmp2 == NULL)
           goto finally;
         tmp2->phone = tmp1->phone;
         tmp2->duration = kInterpolationMargin;
         tmp2->rate = tmp1->rate * kInterpolationRate;
         tmp2->next = tmp1->next;
         tmp1->duration -= kInterpolationMargin;
         tmp1->next = tmp2;
         tmp2 = tmp1;
      }
   }

   /* count length of key frame */
   len = 0;
   for(tmp1 = head; tmp1; tmp1 = tmp1->next)
      len++;

   totalNumKey = m_numMotion * len;

   /* create memories */
   (*rawSize) = sizeof(VMDFile_Header) + sizeof(unsigned int) + sizeof(unsigned int) + sizeof(VMDFile_FaceFrame) * totalNumKey;
   i = (*rawSize);
   i = sizeof(unsigned char) * (*rawSize);
   (*rawData) = static_cast<unsigned char *>(MMDAIMemoryAllocate(i));
   if (*rawData == NULL)
     goto finally;

   data = (*rawData);
   /* header */
   header = (VMDFile_Header *) data;
   MMDAIStringCopy(header->header, "Vocaloid Motion Data 0002", 30);
   data += sizeof(VMDFile_Header);
   /* number of key frame for bone */
   numBoneKeyFrames = (unsigned int *) data;
   (*numBoneKeyFrames) = 0;
   data += sizeof(unsigned int);
   /* number of key frame for expression */
   numFaceKeyFrames = (unsigned int *) data;
   (*numFaceKeyFrames) = totalNumKey;
   data += sizeof(unsigned int);
   /* set key frame */
   for (i = 0; i < m_numMotion; i++) {
      currentFrame = 0;
      for(tmp1 = head; tmp1; tmp1 = tmp1->next) {
         face = (VMDFile_FaceFrame *) data;
         MMDAIStringCopy(face->name, m_motion[i], 15);
         face->keyFrame = currentFrame;
         face->weight = m_blendRate[tmp1->phone][i] * tmp1->rate;
         data += sizeof(VMDFile_FaceFrame);
         currentFrame += tmp1->duration;
      }
   }
   ret = true;

finally:
   /* free */
   if (buf != NULL)
     MMDAIMemoryRelease(buf);
   for(tmp1 = head; tmp1; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      if (tmp1 != NULL)
        MMDAIMemoryRelease(tmp1);
   }
   return ret;
}

const char *LipSync::getMotionName()
{
  return "LipSync";
}

