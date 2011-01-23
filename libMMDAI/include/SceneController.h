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

#ifndef SCENECONTROLLER_H
#define SCENECONTROLLER_H

#include "LipSync.h"
#include "MotionStocker.h"
#include "SceneEventHandler.h"
#include "SceneRenderer.h"

#define MAX_MODEL 20

class SceneController
{
public:
  explicit SceneController(SceneEventHandler *handler);
  ~SceneController();

  void init(int *size, const char *systexPath);

  PMDObject *allocatePMDObject();
  PMDObject *findPMDObject(PMDObject *object);
  PMDObject *findPMDObject(const char *alias);
  PMDObject *getPMDObject(int index);
  int countPMDObjects() const;

  bool loadFloor(const char *fileName);
  bool loadBackground(const char *fileName);
  bool loadStage(PMDModelLoader *loader);

  void updateLight();

  bool addMotion(PMDObject *object, const char *fileName);
  bool addMotion(PMDObject *object,
                 const char *motionAlias,
                 const char *fileName,
                 bool full,
                 bool once,
                 bool enableSmooth,
                 bool enableRePos);
  bool changeMotion(PMDObject *object,
                    const char *motionAlias,
                    const char *fileName);
  bool deleteMotion(PMDObject *object,
                    const char *motionAlias);

  bool addModel(PMDModelLoader *loader);
  bool addModel(const char *modelAlias,
                PMDModelLoader *loader,
                btVector3 *pos,
                btQuaternion *rot,
                const char *baseModelAlias,
                const char *baseBoneName);
  bool changeModel(PMDObject *object, PMDModelLoader *loader);
  void deleteModel(PMDObject *object);

  void changeLightDirection(float x,
                            float y,
                            float z);
  void changeLightColor(float r,
                        float g,
                        float b);

  void startMove(PMDObject *object,
                 btVector3 *pos,
                 bool local,
                 float speed);
  void stopMove(PMDObject *object);

  void startRotation(PMDObject *object,
                     btQuaternion *rot,
                     bool local,
                     float speed);
  void stopRotation(PMDObject *object);

  void startTurn(PMDObject *object,
                 btVector3 *pos,
                 bool local,
                 float speed);
  void stopTurn(PMDObject *object);

  bool startLipSync(PMDObject *object,
                    const char *seq);
  bool stopLipSync(PMDObject *object);

  void getScreenPointPosition(btVector3 *dst, btVector3 *src);
  float getScale();
  void setScale(float value);
  void rotate(float x, float y, float z);
  void translate(float x, float y, float z);
  void setRect(int width, int height);
  void setShadowMapping(bool value);

  void selectPMDObject(PMDObject *object);
  void selectPMDObject(int x, int y);
  void selectPMDObject(int x, int y, PMDObject **dropAllowedModel);
  void hightlightPMDObject();
  PMDObject *getSelectedPMDObject();

  void updateMotion(double procFrame, double adjustFrame);
  void updateDepthTextureViewParam();
  void updateModelPositionAndRotation(double fps);
  void updateAfterSimulation();
  void renderScene();
  void renderBulletForDebug();
  void renderPMDObjectsForDebug();
  void renderLogger();

  const char *getConfigPath();
  Option *getOption();
  Stage *getStage();
  int getWidth();
  int getHeight();

private:
  void deleteAssociatedModels(PMDObject *object);
  void sendEvent1(const char *type, const char *arg1);
  void sendEvent2(const char *type, const char *arg1, const char *arg2);

  BulletPhysics m_bullet;
  MotionStocker m_motion;
  Option m_option;
  PMDObject m_objects[MAX_MODEL];
  SceneEventHandler *m_handler;
  SceneRenderer m_scene;
  Stage m_stage;
  TextRenderer *m_text;
  int m_numModel;
  int m_selectedModel;
  bool m_enablePhysicsSimulation;
};

#endif // SCENECONTROLLER_H
