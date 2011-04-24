/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef MMDAI_SCENECONTROLLER_H_
#define MMDAI_SCENECONTROLLER_H_

#include <MMDME/MMDME.h>

#include "MMDAI/MotionStocker.h"
#include "MMDAI/Stage.h"

namespace MMDAI {

#define MAX_MODEL 20

class ILipSyncLoader;
class PMDObject;
class IPreference;
class SceneEventHandler;
class SceneRenderEngine;

typedef struct RenderDepth RenderDepth;

class SceneController
{
public:
    static const float kRenderViewPointFrustumNear;
    static const float kRenderViewPointFrustumFar;
    static const float kRenderViewPointCameraZ;
    static const float kRenderViewPointYOffset;

    SceneController(SceneEventHandler *handler, IPreference *preference);
    ~SceneController();

    void initializeScreen(int width, int height);

    PMDObject *allocatePMDObject();
    PMDObject *findPMDObject(PMDObject *object);
    PMDObject *findPMDObject(const char *alias);

    bool loadFloor(IModelLoader *loader);
    bool loadBackground(IModelLoader *loader);
    bool loadStage(IModelLoader *loader);

    bool addMotion(PMDObject *object, IMotionLoader *loader);
    bool addMotion(PMDObject *object,
                   const char *motionAlias,
                   IMotionLoader *loader,
                   bool full,
                   bool once,
                   bool enableSmooth,
                   bool enableRePos,
                   float priority);
    bool changeMotion(PMDObject *object,
                      const char *motionAlias,
                      IMotionLoader *loader);
    bool deleteMotion(PMDObject *object,
                      const char *motionAlias);

    bool addModel(IModelLoader *loader, ILipSyncLoader *lipSyncLoader);
    bool addModel(const char *modelAlias,
                  IModelLoader *modelLoader,
                  ILipSyncLoader *lipSyncLoader,
                  btVector3 *pos,
                  btQuaternion *rot,
                  const char *baseModelAlias,
                  const char *baseBoneName);
    bool changeModel(PMDObject *object,
                     IModelLoader *loader,
                     ILipSyncLoader *lipSyncLoader);
    void deleteModel(PMDObject *object);

    void setLightDirection(float x, float y, float z);
    void setLightColor(float r, float g, float b);

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
    void resetLocation(const btVector3 &trans, const float *rot, const float scale);

    void setModelViewPosition(int x, int y);
    void setModelViewRotation(int x, int y);
    void translate(const btVector3 &value);

    void selectPMDObject(PMDObject *object);
    void selectPMDObject(int x, int y);
    void selectPMDObject(int x, int y, PMDObject **dropAllowedModel);
    void deselectPMDObject();
    void setHighlightPMDObject(PMDObject *object);
    void setRect(int width, int height);
    void setViewMoveTimer(int ms);

    void updateLightDirection(float x, float y);
    void updateLight();
    void updateMotion(double procFrame, double adjustFrame);
    void updateDepthTextureViewParam();
    void updateModelPositionAndRotation(double fps);
    void updateAfterSimulation();
    void updateProjection(int ellapsedTimeForMove);
    void updateModelView(int ellapsedTimeForMove);
    void prerenderScene();
    void renderScene();
    void renderBulletForDebug();
    void renderPMDObjectsForDebug();
    void renderLogger();

    inline PMDObject *getPMDObject(int index) const {
        if (index < 0 || index > m_numModel)
            return NULL;
        return m_objects[index];
    }
    inline PMDObject *getSelectedPMDObject() const {
        return getPMDObject(m_selectedModel);
    }
    inline const int countPMDObjects() const {
        return m_numModel;
    }
    inline const btVector3 getScreenPointPosition(const btVector3 &src) {
        return m_transMatrix.inverse() * src;
    }
    inline const float getScale() const {
        return m_scale;
    }
    inline void setScale(float value) {
        m_scale = value;
    }
    inline void setModelView(const btTransform &modelView) {
        m_engine->setModelView(modelView);
    }
    inline void setProjection(const float projection[16]) {
        m_engine->setProjection(projection);
    }
    inline void setShadowMapping() {
        m_engine->setShadowMapping();
    }
    inline Stage *getStage() const {
        return m_stage;
    }
    inline const int getWidth() const {
        return m_width;
    }
    inline const int getHeight() const {
        return m_height;
    }
    inline bool isViewMoving() const {
        return m_viewMoveTime > 0 && (m_currentRot != m_rot || m_currentTrans != m_trans || m_currentScale != m_scale);
    }

private:
    void sortRenderOrder();
    void eraseModel(PMDObject *object);
    void sendEvent1(const char *type, const char *arg1);
    void sendEvent2(const char *type, const char *arg1, const char *arg2);

    BulletPhysics m_bullet;
    SceneRenderEngine *m_engine;
    MotionStocker m_motion;
    PMDObject **m_objects;
    PMDObject *m_highlightModel;
    IPreference *m_preference;
    SceneEventHandler *m_handler;
    Stage *m_stage;
    int m_numModel;
    int m_selectedModel;
    int m_width;
    int m_height;
    bool m_enablePhysicsSimulation;

    float m_scale;           /* target scale */
    btVector3 m_trans;       /* target trans vector */
    btQuaternion m_rot;      /* target rotation */
    btVector3 m_cameraTrans; /* position of camera */

    int m_viewMoveTime;
    btVector3 m_viewMoveStartTrans;
    btQuaternion m_viewMoveStartRot;
    float m_viewMoveStartScale;
    RenderDepth *m_depth;
    int16_t *m_order;

    float m_currentScale;         /* current scale */
    btVector3 m_currentTrans;     /* current trans vector */
    btQuaternion m_currentRot;    /* current rotation */
    btTransform m_transMatrix;    /* current trans vector + rotation matrix */

    MMDME_DISABLE_COPY_AND_ASSIGN(SceneController);
};

} /* namespace */

#endif // MMDAI_SCENECONTROLLER_H_

