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

#ifndef MMDME_VMD_H_
#define MMDME_VMD_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/PTree.h"
#include "MMDME/VMDLoader.h"

#define VMD_INTERPOLATIONTABLESIZE 64 /* motion interpolation table size */

/* BoneKeyFrame: bone key frame */
typedef struct _BoneKeyFrame {
   float keyFrame;               /* key frame */
   btVector3 pos;                /* translation position */
   btQuaternion rot;             /* rotation */
   bool linear[4];               /* this is liner interpolation, use simpler calculation */
   float *interpolationTable[4]; /* table for interpolation */
} BoneKeyFrame;

/* BoneMotion: bone motion unit (list of key frames for a bone defined in a VMD file) */
typedef struct _BoneMotion {
   char *name;                 /* bone name */
   unsigned long numKeyFrame;  /* number of defined key frames */
   BoneKeyFrame *keyFrameList; /* list of key frame data */
} BoneMotion;

/* BoneMotionLink: linked list of defined bone motions in a VMD data */
typedef struct _BoneMotionLink {
   BoneMotion boneMotion; /* bone motion unit */
   struct _BoneMotionLink *next;
} BoneMotionLink;

/* FaceKeyFrame: face key frame */
typedef struct _FaceKeyFrame {
   float keyFrame; /* key frame */
   float weight;   /* face weight */
} FaceKeyFrame;

/* FaceMotion: face motion unit (list of key frames for a face defined in a VMD file) */
typedef struct _FaceMotion {
   char *name;                 /* face name */
   unsigned long numKeyFrame;  /* number of defined key frames */
   FaceKeyFrame *keyFrameList; /* list of key frame data */
} FaceMotion;

/* FaceMotionLink: linked list of defined face motions in a VMD data */
typedef struct _FaceMotionLink {
   FaceMotion faceMotion; /* face motion unit */
   struct _FaceMotionLink *next;
} FaceMotionLink;

/* VMD: motion file class */
class VMD
{
private:

   unsigned long m_numTotalBoneKeyFrame; /* total number of bone frames */
   unsigned long m_numTotalFaceKeyFrame; /* total number of face frames */

   PTree m_name2bone;
   PTree m_name2face;

   BoneMotionLink *m_boneLink; /* linked list of bones in the motion */
   FaceMotionLink *m_faceLink; /* linked list of faces in the motion */

   unsigned long m_numBoneKind; /* number of bones in m_boneLink */
   unsigned long m_numFaceKind; /* number of faces in m_faceLink */

   float m_maxFrame; /* max frame */

   /* addBoneMotion: add new bone motion to list */
   void addBoneMotion(const char *name);

   /* addFaceMotion: add new face motion to list */
   void addFaceMotion(const char *name);

   /* getBoneMotion: find bone motion by name */
   BoneMotion * getBoneMotion(const char *name);

   /* getFaceMotion: find face motion by name */
   FaceMotion * getFaceMotion(const char *name);

   /* setInterpolationTable: set up motion interpolation parameter */
   void setInterpolationTable(BoneKeyFrame *bf, char ip[]);

   /* initialize: initialize VMD */
   void initialize();

   /* clear: free VMD */
   void clear();

public:

   /* VMD: constructor */
   VMD();

   /* ~VMD: destructor */
   ~VMD();

   /* load: initialize and load from file name */
   bool load(VMDLoader *loader);

   /* parse: initialize and load from data memories */
   bool parse(unsigned char *data, size_t size);

   /* getTotalKeyFrame: get total number of key frames */
   unsigned long getTotalKeyFrame() const;

   /* getBoneMotionLink: get list of bone motions */
   BoneMotionLink * getBoneMotionLink() const;

   /* getFaceMotionLink: get list of face motions */
   FaceMotionLink * getFaceMotionLink() const;

   /* getNumBoneKind: get number of bone motions */
   unsigned int getNumBoneKind() const;

   /* getNumFaceKind: get number of face motions */
   unsigned int getNumFaceKind() const;

   /* getMaxFrame: get max frame */
   float getMaxFrame() const;
};

#endif

