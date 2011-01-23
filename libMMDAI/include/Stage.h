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

/* headers */

#ifndef STAGE_H
#define STAGE_H

#include "MMDFiles/MMDFiles.h"

#include "TileTexture.h"

/* Stage: stage */
class Stage
{
public:

  /* Stage: constructor */
  Stage();

  /* ~Stage: destructor */
  ~Stage();

  /* setSize: set size of floor and background */
  void setSize(float *size, float numx, float numy);

  /* loadFloor: load floor image */
  bool loadFloor(PMDModelLoader *loader, BulletPhysics *bullet);

  /* loadBackground: load background image */
  bool loadBackground(PMDModelLoader *loader, BulletPhysics *bullet);

  /* loadStagePMD: load stage pmd */
  bool loadStagePMD(PMDModelLoader *loader, BulletPhysics *bullet);

  /* renderFloor: render the floor */
  void renderFloor();

  /* renderBackground: render the background */
  void renderBackground();

  /* renderPMD: render the stage pmd */
  void renderPMD();

  /* updateShadowMatrix: update shadow projection matrix */
  void updateShadowMatrix(float lightDirection[4]);

  /* getShadowMatrix: get shadow projection matrix */
  GLfloat *getShadowMatrix();

private:
  TileTexture m_floor; /* floor texture */
  TileTexture m_background;  /* background texture */

  PMDModel m_pmd;           /* PMD for background */
  bool m_hasPMD;            /* true if m_pmd is used */
  GLuint m_listIndexPMD;    /* display lst */
  bool m_listIndexPMDValid; /* true if m_listIndexPMDValid was registered */

  BulletPhysics *m_bullet;  /* BulletPhysics */
  btRigidBody *m_floorBody; /* body for floor */

  /* work area */
  GLfloat m_floorShadow[4][4]; /* matrix for shadow of floor */

  /* makeFloorBody: create a rigid body for floor */
  void makeFloorBody(float width, float depth);

  /* releaseFloorBody: release rigid body for floor */
  void releaseFloorBody();

  /* initialize: initialize stage */
  void initialize();

  /* clear: free stage */
  void clear();
};

#endif // STAGE_H
