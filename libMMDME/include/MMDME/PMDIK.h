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

#ifndef MMDME_PMDIK_H_
#define MMDME_PMDIK_H_

#include "MMDME/PMDBone.h"
#include "MMDME/PMDFile.h"

#define PMDIK_PI          3.1415926f
#define PMDIK_MINDISTANCE 0.0001f
#define PMDIK_MINANGLE    0.00000001f
#define PMDIK_MINAXIS     0.0000001f
#define PMDIK_MINROTSUM   0.002f
#define PMDIK_MINROTATION 0.00001f

/* PMDIK: IK for PMD */
class PMDIK
{
private:

   PMDBone *m_destBone;        /* Destination bone. IK tries to move the targetBone to this position */
   PMDBone *m_targetBone;      /* Target bone. IK tries move this bone to the position of destBone */
   PMDBone **m_boneList;       /* List of bones under this IK */
   unsigned char m_numBone;    /* Number of bones under this IK */
   unsigned short m_iteration; /* IK value 1: maximum iteration count */
   float m_angleConstraint;    /* IK value 2: maximum angle per one step in radian */

   /* initialize: initialize IK */
   void initialize();

   /* clear: free IK */
   void clear();

public:

   /* PMDIK: constructor */
   PMDIK();

   /* ~PMDIK: destructor */
   ~PMDIK();

   /* setup: initialize and setup IK */
   void setup(PMDFile_IK *ik, short *ikBoneIDList, PMDBone *boneList);

   /* isSimulated: check if this IK is under simulation, in case no need to calculate this IK */
   bool isSimulated();

   /* solve: try to move targetBone toward destBone, solving constraint among bones in boneList[] and the targetBone */
   void solve();
};

#endif

