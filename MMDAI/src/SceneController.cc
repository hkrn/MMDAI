/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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

#include "MMDAI/MMDAI.h"

#if defined(OPENGLES1)
#include "MMDAI/GLES1SceneRenderEngine.h"
#else
#include "GLSceneRenderEngine.h"
#endif

namespace MMDAI {

/* command names */
const char *ISceneEventHandler::kModelAddCommand = "MODEL_ADD";
const char *ISceneEventHandler::kModelChangeCommand = "MODEL_CHANGE";
const char *ISceneEventHandler::kModelDeleteCommand = "MODEL_DELETE";
const char *ISceneEventHandler::kMotionAddCommand = "MOTION_ADD";
const char *ISceneEventHandler::kMotionChangeCommand = "MOTION_CHANGE";
const char *ISceneEventHandler::kMotionDeleteCommand = "MOTION_DELETE";
const char *ISceneEventHandler::kMoveStartCommand = "MOVE_START";
const char *ISceneEventHandler::kMoveStopCommand = "MOVE_STOP";
const char *ISceneEventHandler::kTurnStartCommand = "TURN_START";
const char *ISceneEventHandler::kTurnStopCommand = "TURN_STOP";
const char *ISceneEventHandler::kRotateStartCommand = "ROTATE_START";
const char *ISceneEventHandler::kRotateStopCommand = "ROTATE_STOP";
const char *ISceneEventHandler::kStageCommand = "STAGE";
const char *ISceneEventHandler::kFloorCommand = "FLOOR";
const char *ISceneEventHandler::kBackgroundCommand = "BACKGROUND";
const char *ISceneEventHandler::kLightColorCommand = "LIGHTCOLOR";
const char *ISceneEventHandler::kLightDirectionCommand = "LIGHTDIRECTION";
const char *ISceneEventHandler::kLipSyncStartCommand = "LIPSYNC_START";
const char *ISceneEventHandler::kLipSyncStopCommand = "LIPSYNC_STOP";
const char *ISceneEventHandler::kCameraCommand = "CAMERA";

const char *ISceneEventHandler::kModelAddEvent = "MODEL_EVENT_ADD";
const char *ISceneEventHandler::kModelChangeEvent = "MODEL_EVENT_CHANGE";
const char *ISceneEventHandler::kModelDeleteEvent = "MODEL_EVENT_DELETE";
const char *ISceneEventHandler::kMotionAddEvent = "MOTION_EVENT_ADD";
const char *ISceneEventHandler::kMotionDeleteEvent = "MOTION_EVENT_DELETE";
const char *ISceneEventHandler::kMotionChangeEvent = "MOTION_EVENT_CHANGE";
const char *ISceneEventHandler::kMotionLoopEvent = "MOTION_EVENT_LOOP";
const char *ISceneEventHandler::kMoveStartEvent = "MOVE_EVENT_START";
const char *ISceneEventHandler::kMoveStopEvent = "MOVE_EVENT_STOP";
const char *ISceneEventHandler::kTurnStartEvent = "TURN_EVENT_START";
const char *ISceneEventHandler::kTurnStopEvent = "TURN_EVENT_STOP";
const char *ISceneEventHandler::kRotateStartEvent = "ROTATE_EVENT_START";
const char *ISceneEventHandler::kRotateStopEvent = "ROTATE_EVENT_STOP";
const char *ISceneEventHandler::kStageEvent = "STAGE";
const char *ISceneEventHandler::kFloorEvent = "FLOOR";
const char *ISceneEventHandler::kBackgroundEvent = "BACKGROUND";
const char *ISceneEventHandler::kLightColorEvent = "LIGHTCOLOR";
const char *ISceneEventHandler::kLightDirectionEvent = "LIGHTDIRECTION";
const char *ISceneEventHandler::kLipSyncStartEvent = "LIPSYNC_EVENT_START";
const char *ISceneEventHandler::kLipSyncStopEvent = "LIPSYNC_EVENT_STOP";
const char *ISceneEventHandler::kKeyEvent = "KEY";

const float SceneController::kRenderViewPointFrustumNear = 0.5f;
const float SceneController::kRenderViewPointFrustumFar = 8000.0f;
const float SceneController::kRenderViewPointCameraZ = -100.0f;
const float SceneController::kRenderViewPointYOffset = -13.0f;

#define RENDER_MINMOVEDIFF       0.000001f
#define RENDER_MOVESPEEDRATE     0.9f
#define RENDER_MINSPINDIFF       0.000001f
#define RENDER_SPINSPEEDRATE     0.9f
#define RENDER_MINDISTANCEDIFF   0.1f
#define RENDER_DISTANCESPEEDRATE 0.9f
#define RENDER_MINFOVYDIFF       0.01f
#define RENDER_FOVYSPEEDRATE     0.9f

struct RenderDepth {
    float distance;
    short id;
};

static int compareDepth(const void *a, const void *b)
{
    const RenderDepth *x = static_cast<const RenderDepth *>(a);
    const RenderDepth *y = static_cast<const RenderDepth *>(b);
    if (x->distance == y->distance)
        return 0;
    return x->distance > y->distance ? 1 : -1;
}

// from util.h
static int getNumDigit(int in)
{
    int out = 0;
    if(in == 0)
        return 1;
    if (in < 0) {
        out = 1;
        in *= -1;
    }
    for (; in != 0; out++)
        in /= 10;
    return out;
}

SceneController::SceneController(ISceneEventHandler *handler, IPreference *preference)
    : m_engine(0),
      m_objects(0),
      m_highlightModel(0),
      m_preference(preference),
      m_handler(handler),
      m_stage(0),
      m_maxModel(0),
      m_numModel(0),
      m_selectedModel(0),
      m_width(0),
      m_height(0),
      m_cameraControlled(false),
      m_enablePhysicsSimulation(true),
      m_viewMoveTime(-1),
      m_viewControlledByMotion(false),
      m_viewMoveStartTrans(0.0f, 0.0f, 0.0f),
      m_viewMoveStartRot(0.0f, 0.0f, 0.0f, 1.0f),
      m_viewMoveStartDistance(0.0f),
      m_viewMoveStartFovy(0.0f),
      m_depth(0),
      m_order(0),
      m_trans(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_rot(0.0f, 0.0f, 0.0f, 1.0f),
      m_distance(preference->getFloat(kPreferenceCameraDistance)),
      m_fovy(preference->getFloat(kPreferenceCameraFovy)),
      m_currentTrans(m_trans),
      m_currentRot(m_rot),
      m_currentDistance(m_distance),
      m_currentFovy(m_fovy)
{
    int maxModel = preference->getInt(kPreferenceMaxModelSize);
    if (maxModel <= 0)
        maxModel = 20; /* for compatibility */
    m_engine = new GLSceneRenderEngine(preference);
    m_objects = new PMDObject*[maxModel];
    m_depth = new RenderDepth[maxModel];
    m_order = new int16_t[maxModel];
    m_stage = new Stage(m_engine);
    m_maxModel = maxModel;
    for (int i = 0; i < maxModel; i++) {
        RenderDepth *depth = &m_depth[i];
        m_objects[i] = new PMDObject(m_engine);
        m_order[i] = 0;
        depth->id = 0;
        depth->distance = 0;
    }
    updateModelViewMatrix();
}

SceneController::~SceneController()
{
    m_cameraControlled = false;
    m_enablePhysicsSimulation = false;
    m_viewControlledByMotion = false;
    m_height = 0;
    m_width = 0;
    m_selectedModel = -1;
    m_numModel = 0;
    m_highlightModel = 0;
    for (int i = 0; i < m_maxModel; i++) {
        delete m_objects[i];
        m_objects[i] = 0;
    }
    m_maxModel = 0;
    delete[] m_order;
    delete[] m_depth;
    delete[] m_objects;
    m_objects = 0;
    delete m_stage;
    m_stage = 0;
    delete m_engine;
    m_engine = 0;
    m_handler = 0;
    m_preference = 0;
}

void SceneController::initialize(int width, int height)
{
    float size[3];
    setRect(width, height);
    m_bullet.setup(m_preference->getInt(kPreferenceBulletFPS));
    m_engine->setup();
    m_preference->getFloat3(kPreferenceStageSize, size);
    m_stage->setSize(size, 1.0f, 1.0f);
    float rot[3], trans[3];
    m_preference->getFloat3(kPreferenceCameraRotation, rot);
    m_preference->getFloat3(kPreferenceCameraTransition, trans);
    float distance = m_preference->getFloat(kPreferenceCameraDistance);
    float fovy = m_preference->getFloat(kPreferenceCameraFovy);
    resetCamera(btVector3(trans[0], trans[1], trans[2]), btVector3(rot[0], rot[1], rot[2]), distance, fovy);
    setViewMoveTimer(0);
}

void SceneController::updateLight()
{
    int i = 0;
    float direction[4];
    m_engine->updateLighting();
    m_preference->getFloat4(kPreferenceLightDirection, direction);
    m_stage->updateShadowMatrix(direction);
    btVector3 dir = btVector3(direction[0], direction[1], direction[2]);
    for (i = 0; i < m_maxModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable())
            object->setLightForToon(dir);
    }
}

bool SceneController::loadFloor(IModelLoader *loader)
{
    /* load floor */
    const char *fileName = loader->getLocation();
    if (fileName == NULL)
        return false;
    if (m_stage->loadFloor(loader, &m_bullet) == false) {
        MMDAILogWarn("cannot set floor %s.", fileName);
        return false;
    }

    /* send event message */
    sendEvent1(ISceneEventHandler::kFloorEvent, fileName);

    return true;
}

bool SceneController::loadBackground(IModelLoader *loader)
{
    /* load background */
    const char *fileName = loader->getLocation();
    if (fileName == NULL)
        return false;
    if (m_stage->loadBackground(loader, &m_bullet) == false) {
        MMDAILogWarn("cannot set background %s.", fileName);
        return false;
    }

    /* send event message */
    sendEvent1(ISceneEventHandler::kBackgroundEvent, fileName);

    return true;
}

bool SceneController::loadStage(IModelLoader *loader)
{
    /* load stage */
    const char *fileName = loader->getLocation();
    if (fileName == NULL)
        return false;
    if (m_stage->loadStagePMD(loader, &m_bullet) == false) {
        MMDAILogWarn("cannot set stage %s.", fileName);
        return false;
    }

    /* send event message */
    sendEvent1(ISceneEventHandler::kStageEvent, fileName);

    return true;
}

PMDObject *SceneController::allocateObject()
{
    PMDObject *object = NULL;
    for (int i = 0; i < m_maxModel; i++) {
        object = m_objects[i];
        if (!object->isEnable())
            return object; /* re-use it */
    }
    if (m_numModel >= m_maxModel)
        return object; /* no more model */
    object = m_objects[m_numModel];
    object->setEnable(false); /* model is not loaded yet */
    return object;
}

PMDObject *SceneController::findObject(PMDObject *object)
{
    return findObject(object->getAlias());
}

PMDObject *SceneController::findObject(const char *alias)
{
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable() && MMDAIStringEquals(object->getAlias(), alias))
            return object;
    }
    return NULL;
}

bool SceneController::addMotion(PMDObject *object,
                                const char *motionAlias,
                                IMotionLoader *loader,
                                bool full,
                                bool once,
                                bool enableSmooth,
                                bool enableRePos,
                                float priority)
{
    char *name;

    /* motion file */
    VMD *vmd = m_motion.loadFromLoader(loader);
    if (vmd == NULL) {
        MMDAILogWarn("addMotion: failed to load %s.", loader->getLocation());
        return false;
    }

    /* alias */
    if (motionAlias && MMDAIStringLength(motionAlias) > 0) {
        /* check the same alias */
        name = MMDAIStringClone(motionAlias);
        MotionPlayer *motionPlayer = object->getMotionManager()->getMotionPlayerList();
        for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
            if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, name)) {
                MMDAILogWarn("addMotion: motion alias \"%s\" is already used.", name);
                MMDAIMemoryRelease(name);
                return false;
            }
        }
    } else {
        /* if motion alias is not specified, unused digit is used */
        for (int i = 0;; i++) {
            bool find = false;
            size_t allocSize = sizeof(char) * (getNumDigit(i) + 1);
            name = static_cast<char *>(MMDAIMemoryAllocate(allocSize));
            if (name == NULL)
                return false;
            MMDAIStringFormat(name, allocSize, "%d", i);
            MotionPlayer *motionPlayer = object->getMotionManager()->getMotionPlayerList();
            for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
                if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, name)) {
                    find = true;
                    break;
                }
            }
            if(find == false)
                break;
            MMDAIMemoryRelease(name);
        }
    }

    /* start motion */
    if (!object->startMotion(vmd, name, full, once, enableSmooth, enableRePos, priority)) {
        MMDAIMemoryRelease(name);
        return false;
    }

    /* send event message */
    sendEvent2(ISceneEventHandler::kMotionAddEvent, object->getAlias(), name);

    MMDAIMemoryRelease(name);
    return true;
}

bool SceneController::changeMotion(PMDObject *object, const char *motionAlias, IMotionLoader *loader)
{
    VMD *old = NULL;

    /* check */
    if (!motionAlias) {
        MMDAILogWarn("changeMotion: not specified %s.", motionAlias);
        return false;
    }

    /* motion file */
    VMD *vmd = m_motion.loadFromLoader(loader);
    if (vmd == NULL) {
        MMDAILogWarn("changeMotion: failed to load %s.", loader->getLocation());
        return false;
    }

    /* get motion before change */
    MotionPlayer *motionPlayer = object->getMotionManager()->getMotionPlayerList();
    for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
        if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, motionAlias)) {
            old = motionPlayer->vmd;
            break;
        }
    }
    if (old == NULL) {
        MMDAILogWarn("changeMotion: motion alias \"%s\"is not found.", motionAlias);
        m_motion.unload(vmd);
        return false;
    }

    /* change motion */
    if (object->swapMotion(vmd, motionAlias) == false) {
        MMDAILogWarn("changeMotion: motion alias \"%s\"is not found.", motionAlias);
        m_motion.unload(vmd);
        return false;
    }

    /* send event message */
    sendEvent2(ISceneEventHandler::kMotionChangeEvent, object->getAlias(), motionAlias);

    /* unload old motion from motion stocker */
    m_motion.unload(old);

    return true;
}

bool SceneController::deleteMotion(PMDObject *object, const char *motionAlias)
{
    /* delete motion */
    if (!object->getMotionManager()->deleteMotion(motionAlias)) {
        MMDAILogWarn("deleteMotion: motion alias \"%s\"is not found.", motionAlias);
        return false;
    }

    /* don't send event message */
    return true;
}

bool SceneController::addModel(const char *modelAlias,
                               IModelLoader *modelLoader,
                               ILipSyncLoader *lipSyncLoader,
                               btVector3 *pos,
                               btQuaternion *rot,
                               const char *baseModelAlias,
                               const char *baseBoneName)
{
    char *name;
    btVector3 offsetPos(0.0f, 0.0f, 0.0f);
    btQuaternion offsetRot(0.0f, 0.0f, 0.0f, 1.0f);
    bool forcedPosition = false;
    PMDBone *assignBone = NULL;
    PMDObject *assignObject = NULL, *newObject = NULL;
    float direction[4];

    m_preference->getFloat4(kPreferenceLightDirection, direction);
    btVector3 light = btVector3(direction[0], direction[1], direction[2]);

    /* set */
    if (pos)
        offsetPos = (*pos);
    if (rot)
        offsetRot = (*rot);
    if (pos || rot)
        forcedPosition = true;
    if (baseModelAlias) {
        assignObject = findObject(baseModelAlias);
        if (assignObject == NULL) {
            MMDAILogWarn("model alias \"%s\" is not found", baseModelAlias);
            return false;
        }
        PMDModel *model = assignObject->getModel();
        if (baseBoneName) {
            assignBone = model->getBone(baseBoneName);
        } else {
            assignBone = model->getCenterBone();
        }
        if (assignBone == NULL) {
            if (baseBoneName)
                MMDAILogWarn("bone \"%s\" is not exist on %s.", baseBoneName, baseModelAlias);
            else
                MMDAILogWarn("cannot assign to bone of %s.", baseModelAlias);
            return false;
        }
    }

    /* ID */
    newObject = allocateObject();
    if (newObject == NULL) {
        MMDAILogWarnString("addModel: too many models.");
        return false;
    }

    /* determine name */
    if (modelAlias && MMDAIStringLength(modelAlias) > 0) {
        /* check the same alias */
        name = MMDAIStringClone(modelAlias);
        if (findObject(name) != NULL) {
            MMDAILogWarn("addModel: model alias \"%s\" is already used.", name);
            MMDAIMemoryRelease(name);
            return false;
        }
    } else {
        /* if model alias is not specified, unused digit is used */
        for(int i = 0;; i++) {
            size_t allocSize = sizeof(char) * (getNumDigit(i) + 1);
            name = static_cast<char *>(MMDAIMemoryAllocate(allocSize));
            if (name == NULL)
                return false;
            MMDAIStringFormat(name, allocSize, "%d", i);
            if (findObject(name) != NULL)
                MMDAIMemoryRelease(name);
            else
                break;
        }
    }

    /* add model */
    if (!newObject->load(modelLoader,
                         lipSyncLoader,
                         &m_bullet,
                         assignBone,
                         assignObject,
                         offsetPos,
                         offsetRot,
                         forcedPosition)) {
        MMDAILogWarn("addModel: failed to load %s.", modelLoader->getLocation());
        newObject->release();
        MMDAIMemoryRelease(name);
        return false;
    }

    PMDModel *model = newObject->getModel();
    model->setToonEnable(m_preference->getBool(kPreferenceUseCartoonRendering));
    model->setEdgeThin(m_preference->getFloat(kPreferenceCartoonEdgeWidth));
    newObject->setLightForToon(light);

    /* initialize motion manager */
    newObject->resetMotionManager();
    newObject->updateRootBone();
    newObject->updateMotion(0.0f);
    newObject->updateAlpha(0.0f);
    newObject->setPhysicsEnable(m_enablePhysicsSimulation);
    newObject->updateSkin();
    newObject->setAlias(name);
    m_numModel++;

    /* send event message */
    sendEvent1(ISceneEventHandler::kModelAddEvent, name);

    MMDAIMemoryRelease(name);
    return true;
}

bool SceneController::changeModel(PMDObject *object,
                                  IModelLoader *modelLoader,
                                  ILipSyncLoader *lipSyncLoader)
{
    static const btVector3 trans(0.0f, 0.0f, 0.0f);
    static const btQuaternion rot(0.0f, 0.0f, 0.0f, 1.0f);
    const char *modelAlias = object->getAlias();
    float direction[4];

    m_preference->getFloat4(kPreferenceLightDirection, direction);
    btVector3 light = btVector3(direction[0], direction[1], direction[2]);

    /* load model */
    if (!object->load(modelLoader,
                      lipSyncLoader,
                      &m_bullet,
                      NULL,
                      NULL,
                      trans,
                      rot,
                      false)) {
        MMDAILogWarn("changeModel: failed to load model %s.", modelLoader->getLocation());
        return false;
    }

    PMDModel *model = object->getModel();
    model->setToonEnable(m_preference->getBool(kPreferenceUseCartoonRendering));
    model->setEdgeThin(m_preference->getFloat(kPreferenceCartoonEdgeWidth));
    object->setLightForToon(light);

    /* update motion manager */
    MotionManager *manager = object->getMotionManager();
    if (manager != NULL) {
        MotionPlayer *motionPlayer = manager->getMotionPlayerList();
        for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
            if (motionPlayer->active) {
                double currentFrame = motionPlayer->mc.getCurrentFrame();
                manager->startMotionSub(motionPlayer->vmd, motionPlayer);
                motionPlayer->mc.setCurrentFrame(currentFrame);
            }
        }
    }

    object->updateRootBone();
    object->updateMotion(0.0f);
    object->updateAlpha(0.0f);
    object->setPhysicsEnable(m_enablePhysicsSimulation);
    object->updateSkin();
    /* set alias */
    object->setAlias(modelAlias);

    /* delete accessories  */
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *assoc = m_objects[i];
        if (assoc->isEnable() && assoc->getAssignedModel() == object) {
            deleteModel(assoc);
        }
    }

    /* send event message */
    sendEvent1(ISceneEventHandler::kModelChangeEvent, modelAlias);

    return true;
}

void SceneController::deleteModel(PMDObject *object)
{
    /* delete accessories  */
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *assoc = m_objects[i];
        if (assoc->isEnable() && assoc->getAssignedModel() == object) {
            deleteModel(assoc);
        }
    }

    /* set frame from now to disappear */
    object->startDisappear();

    /* send event message */
    sendEvent1(ISceneEventHandler::kModelDeleteEvent, object->getAlias());
}

void SceneController::setLightDirection(float x, float y)
{
    float direction[4];
    float step = m_preference->getFloat(kPreferenceRotateStep) * 0.1f;
    m_preference->getFloat4(MMDAI::kPreferenceLightDirection, direction);
    btVector3 v = btVector3(direction[0], direction[1], direction[2]);
    btMatrix3x3 matrix = btMatrix3x3(btQuaternion(0, y * MMDAIMathRadian(step), 0.0) * btQuaternion(x * MMDAIMathRadian(step), 0, 0));
    v = v * matrix;
    setLightDirection(v);
}

void SceneController::setLightDirection(const btVector3 &direction)
{
    float f[4] = { direction.x(), direction.y(), direction.z(), 0.0f };
    m_preference->setFloat4(kPreferenceLightDirection, f);
    updateLight();

    /* send event message */
    if (m_handler != NULL) {
        char buf[BUFSIZ];
        MMDAIStringFormatSafe(buf, sizeof(buf), "%.2f,%.2f,%.2f", f[0], f[1], f[2]);
        sendEvent1(ISceneEventHandler::kLightDirectionEvent, buf);
    }
}

void SceneController::setLightColor(const btVector3 &color)
{
    float f[3] = { color.x(), color.y(), color.z() };
    m_preference->setFloat3(kPreferenceLightColor, f);
    updateLight();

    /* send event message */
    if (m_handler != NULL) {
        char buf[BUFSIZ];
        MMDAIStringFormatSafe(buf, sizeof(buf), "%.2f,%.2f,%.2f", f[0], f[1], f[2]);
        sendEvent1(ISceneEventHandler::kLightColorEvent, buf);
    }
}

void SceneController::startMove(PMDObject *object, const btVector3 &pos, bool local, float speed)
{
    const char *alias = object->getAlias();

    if(object->isMoving())
        sendEvent1(ISceneEventHandler::kMoveStopEvent, alias);

    /* get */
    const btVector3 currentPos = object->getCurrentPosition();
    btVector3 targetPos;

    /* local or global */
    if (local) {
        const btQuaternion currentRot = object->getCurrentRotation();
        btTransform tr = btTransform(currentRot, currentPos);
        targetPos = tr * pos;
    }
    else {
        targetPos = pos;
    }

    /* not need to start */
    if (currentPos == targetPos) {
        sendEvent1(ISceneEventHandler::kMoveStartEvent, alias);
        sendEvent1(ISceneEventHandler::kMoveStopEvent, alias);
        return;
    }

    object->setMoveSpeed(speed);
    object->setPosition(targetPos);

    /* send event message */
    sendEvent1(ISceneEventHandler::kMoveStartEvent, alias);
}

void SceneController::stopMove(PMDObject *object)
{
    const char *alias = object->getAlias();

    if (!object->isMoving()) {
        MMDAILogWarn("stopMove: %s is not moving", alias);
        return;
    }

    const btVector3 currentPos = object->getCurrentPosition();
    object->setPosition(currentPos);

    /* send event message */
    sendEvent1(ISceneEventHandler::kMoveStopEvent, alias);
}

void SceneController::startRotation(PMDObject *object, const btQuaternion &rot, bool local, float speed)
{
    const char *alias = object->getAlias();

    if (object->isRotating()) {
        if (object->isTurning())
            sendEvent1(ISceneEventHandler::kTurnStopEvent, alias);
        else
            sendEvent1(ISceneEventHandler::kRotateStopEvent, alias);
    }

    const btQuaternion currentRot = object->getCurrentRotation();
    const btQuaternion targetRot = local ? currentRot * rot : currentRot.nearest(rot);

    /* not need to start */
    if (currentRot == targetRot) {
        sendEvent1(ISceneEventHandler::kRotateStartEvent, alias);
        sendEvent1(ISceneEventHandler::kRotateStopEvent, alias);
        return;
    }

    object->setSpinSpeed(speed);
    object->setRotation(targetRot);
    object->setTurning(false);

    sendEvent1(ISceneEventHandler::kRotateStartEvent, alias);
}

void SceneController::stopRotation(PMDObject *object)
{
    const char *alias = object->getAlias();

    if (!object->isRotating() || object->isTurning()) {
        MMDAILogWarn("stopTurn: %s is not rotating", alias);
        return;
    }

    const btQuaternion currentRot = object->getCurrentRotation();
    object->setRotation(currentRot);

    sendEvent1(ISceneEventHandler::kRotateStopEvent, alias);
}

void SceneController::startTurn(PMDObject *object, const btVector3 &pos, bool local, float speed)
{
    btQuaternion targetRot;
    const char *alias = object->getAlias();

    if (object->isRotating()) {
        if (object->isTurning())
            sendEvent1(ISceneEventHandler::kTurnStopEvent, alias);
        else
            sendEvent1(ISceneEventHandler::kRotateStopEvent, alias);
    }

    const btVector3 currentPos = object->getCurrentPosition();
    const btQuaternion currentRot = object->getCurrentRotation();

    btVector3 targetPos = local ? pos : pos - currentPos;
    targetPos.normalize();

    /* calculate target rotation from (0,0,1) */
    float z = targetPos.z();
    if (z > 1.0f)
        z = 1.0f;
    if (z < -1.0f)
        z = -1.0f;
    float rad = acosf(z);
    btVector3 axis = btVector3(0.0f, 0.0f, 1.0f).cross(targetPos);
    if (axis.length2() < PMDOBJECT_MINSPINDIFF) {
        targetRot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else {
        axis.normalize();
        targetRot = btQuaternion(axis, btScalar(rad));
    }
    targetRot = local ? currentRot * targetRot : currentRot.nearest(targetRot);

    if (currentRot == targetRot) {
        sendEvent1(ISceneEventHandler::kTurnStartEvent, alias);
        sendEvent1(ISceneEventHandler::kTurnStopEvent, alias);
        return;
    }

    object->setSpinSpeed(speed);
    object->setRotation(targetRot);
    object->setTurning(true);

    /* send event message */
    sendEvent1(ISceneEventHandler::kTurnStartEvent, alias);
}

void SceneController::stopTurn(PMDObject *object)
{
    const char *alias = object->getAlias();

    if (!object->isRotating() || !object->isTurning()) {
        MMDAILogWarn("stopTurn: %s is not turning", alias);
        return;
    }

    const btQuaternion currentRot = object->getCurrentRotation();
    object->setRotation(currentRot);

    /* send event message */
    sendEvent1(ISceneEventHandler::kTurnStopEvent, alias);
}

bool SceneController::startLipSync(PMDObject *object, const char *seq)
{
    unsigned char *vmdData = NULL;
    size_t vmdSize = 0;

    /* create motion */
    if(object->createLipSyncMotion(seq, &vmdData, &vmdSize) == false) {
        MMDAILogWarnString("cannot create lip motion.");
        return false;
    }
    VMD *vmd = m_motion.loadFromData(vmdData, vmdSize);
    MMDAIMemoryRelease(vmdData);

    /* search running lip motion */
    bool found = false;
    const char *name = LipSync::getMotionName();
    MotionPlayer *motionPlayer = object->getMotionManager()->getMotionPlayerList();
    for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
        if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, name)) {
            found = true;
            break;
        }
    }

    /* start lip sync */
    if (found) {
        if (!object->swapMotion(vmd, name)) {
            MMDAILogWarnString("cannot start lip sync.");
            m_motion.unload(vmd);
            return false;
        }
    } else {
        if (!object->startMotion(vmd, name, false, true, true, true, MotionManager::kDefaultPriority)) {
            MMDAILogWarnString("cannot start lip sync.");
            m_motion.unload(vmd);
            return false;
        }
    }

    /* send event message */
    sendEvent1(ISceneEventHandler::kLipSyncStartEvent, object->getAlias());

    return true;
}

bool SceneController::stopLipSync(PMDObject *object)
{
    /* stop lip sync */
    if (object->getMotionManager()->deleteMotion(LipSync::getMotionName()) == false) {
        MMDAILogWarnString("lipsync motion not found");
        return false;
    }

    /* don't send message */
    return true;
}

void SceneController::resetCamera(const btVector3 &trans, const btVector3 &angle, const float distance, const float fovy)
{
    m_trans = trans;
    m_angle = angle;
    m_distance = distance;
    m_fovy = fovy;
    updateRotationFromAngle();
}

void SceneController::loadCameraMotion(IMotionLoader *motionLoader)
{
    if (!m_cameraControlled && m_cameraMotion.load(motionLoader)) {
        m_cameraController.setup(&m_cameraMotion);
        m_cameraController.reset();
        m_cameraControlled = true;
        m_viewControlledByMotion = true;
    }
}

void SceneController::setModelViewPosition(int x, int y)
{
    float step = 0.0005 * m_distance;
    btVector3 v(x * step, -y * step, 0.0f);
    btTransform tr(m_transMatrix);
    tr.setOrigin(btVector3(0.0f, 0.0f, 0.0f));
    v = tr.inverse() * v;
    translate(v * -1.0f);
}

void SceneController::setModelViewRotation(int x, int y)
{
    float step = m_preference->getFloat(MMDAI::kPreferenceRotateStep) * 0.1f;
    m_angle.setX(m_angle.x() + x * step);
    m_angle.setY(m_angle.y() + y * step);
    updateRotationFromAngle();
}

void SceneController::translate(const btVector3 &value)
{
    m_trans += value;
}

void SceneController::setRect(int width, int height)
{
    if (m_width != width || m_height != height) {
        if (width > 0)
            m_width = width;
        if (height > 0)
            m_height = height;
        updateProjectionMatrix();
    }
}

void SceneController::setViewMoveTimer(int ms)
{
    m_viewMoveTime = ms;
    if (m_viewMoveTime > 0) {
        m_viewMoveStartRot = m_currentRot;
        m_viewMoveStartTrans = m_currentTrans;
        m_viewMoveStartDistance = m_currentDistance;
        m_viewMoveStartFovy = m_currentFovy;
    }
}

void SceneController::selectObject(PMDObject *object)
{
    assert(object != NULL);
    const char *alias = object->getAlias();
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *o = m_objects[i];
        if (o->isEnable() && MMDAIStringEquals(o->getAlias(), alias)) {
            m_selectedModel = i;
            break;
        }
    }
}

void SceneController::selectObject(int x, int y)
{
    m_selectedModel = m_engine->pickModel(m_objects,
                                          m_numModel,
                                          x,
                                          y,
                                          NULL);
}

void SceneController::selectObject(int x, int y, PMDObject **dropAllowedModel)
{
    int dropAllowedModelID = -1;
    m_selectedModel = m_engine->pickModel(m_objects,
                                          m_numModel,
                                          x,
                                          y,
                                          &dropAllowedModelID);
    if (m_selectedModel == -1)
        *dropAllowedModel = getObjectAt(dropAllowedModelID);
}

void SceneController::deselectObject()
{
    m_selectedModel = -1;
}

void SceneController::setHighlightObject(PMDObject *object)
{
    float col[4];

    if (m_highlightModel != NULL) {
        /* reset current highlighted model */
        col[0] = PMDModel::kEdgeColorR;
        col[1] = PMDModel::kEdgeColorG;
        col[2] = PMDModel::kEdgeColorB;
        col[3] = PMDModel::kEdgeColorA;
        PMDModel *model = m_highlightModel->getModel();
        if (model)
            model->setEdgeColor(col);
    }
    if (object != NULL) {
        /* set highlight to the specified model */
        m_preference->getFloat4(kPreferenceCartoonEdgeSelectedColor, col);
        object->getModel()->setEdgeColor(col);
    }

    m_highlightModel = object;
}

void SceneController::updateMotion(double procFrame, double adjustFrame)
{
    const char *lipSyncMotion = LipSync::getMotionName();
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable()) {
            object->updateRootBone();
            if (object->updateMotion(procFrame + adjustFrame)) {
                MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
                for (; player != NULL; player = player->next) {
                    if (player->statusFlag == MOTION_STATUS_DELETED) {
                        if (MMDAIStringEquals(player->name, lipSyncMotion)) {
                            sendEvent1(ISceneEventHandler::kLipSyncStopEvent, object->getAlias());
                        }
                        else {
                            sendEvent2(ISceneEventHandler::kMotionDeleteEvent, object->getAlias(), player->name);
                        }
                        m_motion.unload(player->vmd);
                    }
                }
            }
            // the model is transparent, model will be deleted
            if (object->updateAlpha(procFrame + adjustFrame)) {
                eraseModel(object);
            }
        }
    }
    m_bullet.update((float)procFrame);
    if (m_cameraControlled) {
        // the camera motion reached to the end, camera controll will be disabled
        if (m_cameraController.advance(procFrame + adjustFrame)) {
            m_cameraControlled = false;
            m_viewControlledByMotion = false;
        }
        m_cameraController.getCurrentViewParam(&m_distance, &m_trans, &m_angle, &m_fovy);
        updateRotationFromAngle();
    }
}

void SceneController::eraseModel(PMDObject *object)
{
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *assoc = m_objects[i];
        // associated accessories (models) is found, these are also deleted.
        if (assoc->isEnable() && assoc->getAssignedModel() == object) {
            eraseModel(assoc);
        }
    }
    const char *alias = object->getAlias();
    const char *lipSyncMotion = LipSync::getMotionName();
    MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
    for (; player != NULL; player = player->next) {
        if (MMDAIStringEquals(player->name, lipSyncMotion)) {
            sendEvent1(ISceneEventHandler::kLipSyncStopEvent, alias);
        }
        else {
            sendEvent2(ISceneEventHandler::kMotionDeleteEvent, alias, player->name);
        }
        m_motion.unload(player->vmd);
    }
    m_numModel--;
    object->release();
}

void SceneController::updateSkin()
{
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *object = m_objects[i];
        object->setPhysicsEnable(m_enablePhysicsSimulation);
        object->updateSkin();
    }
}

void SceneController::updateDepthTextureViewParam()
{
    /* calculate rendering range for shadow mapping */
    if (m_preference->getBool(kPreferenceUseShadowMapping)) {
        int num = m_maxModel;
        float d = 0, dmax = 0;
        float *r = new float[num];
        btVector3 *c = new btVector3[num];
        btVector3 cc = btVector3(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < num; i++) {
            PMDObject *object = m_objects[i];
            if (!object->isEnable())
                continue;
            r[i] = object->getModel()->calculateBoundingSphereRange(&(c[i]));
            cc += c[i];
        }
        if (num != 0)
            cc /= static_cast<float>(num);

        dmax = 0.0f;
        for (int i = 0; i < num; i++) {
            if (!m_objects[i]->isEnable())
                continue;
            d = cc.distance(c[i]) + r[i];
            if (dmax < d)
                dmax = d;
        }
        m_engine->setShadowMapAutoView(cc, dmax);

        delete [] r;
        delete [] c;
    }
}

void SceneController::updateObject(double fps)
{
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable()) {
            const char *alias = object->getAlias();
            if (object->move(fps)) {
                sendEvent1(ISceneEventHandler::kMoveStopEvent, alias);
            }
            if (object->rotate(fps)) {
                if (object->isTurning()) {
                    sendEvent1(ISceneEventHandler::kTurnStopEvent, alias);
                    object->setTurning(false);
                } else {
                    sendEvent1(ISceneEventHandler::kRotateStopEvent, alias);
                }
            }
        }
    }
}

inline void SceneController::sendEvent1(const char *type, const char *arg1)
{
    if (m_handler != NULL)
        m_handler->handleEventMessage(type, 1, arg1);
}

inline void SceneController::sendEvent2(const char *type, const char *arg1, const char *arg2)
{
    if (m_handler != NULL)
        m_handler->handleEventMessage(type, 2, arg1, arg2);
}

void SceneController::updateModelView(int ellapsedTimeForMove)
{
    if  (updateDistance(ellapsedTimeForMove) || m_currentRot != m_rot || m_currentTrans != m_trans) {
        if (m_viewMoveTime == 0 || m_viewControlledByMotion) {
            /* immediately apply the target */
            m_currentRot = m_rot;
            m_currentTrans = m_trans;
        }
        else if (m_viewMoveTime > 0) {
            /* constant move */
            if (ellapsedTimeForMove >= m_viewMoveTime) {
                m_currentRot = m_rot;
                m_currentTrans = m_trans;
            }
            else {
                m_currentTrans = m_viewMoveStartTrans.lerp(m_trans, ellapsedTimeForMove / m_viewMoveTime);
                m_currentRot = m_viewMoveStartRot.slerp(m_rot, ellapsedTimeForMove / m_viewMoveTime);
            }
        }
        else {
            /* calculate difference */
            btVector3 trans = m_trans;
            trans -= m_currentTrans;
            float diff1 = trans.length2();
            btQuaternion rot = m_rot;
            rot -= m_currentRot;
            float diff2 = rot.length2();
            if (diff1 > RENDER_MINMOVEDIFF)
                m_currentTrans = m_currentTrans.lerp(m_trans, 1.0f - RENDER_MOVESPEEDRATE); /* current * 0.9 + target * 0.1 */
            else
                m_currentTrans = m_trans;
            if (diff2 > RENDER_MINSPINDIFF)
                m_currentRot = m_currentRot.slerp(m_rot, 1.0f - RENDER_SPINSPEEDRATE); /* current * 0.9 + target * 0.1 */
            else
                m_currentRot = m_rot;
        }
        updateModelViewMatrix();
    }
}

void SceneController::updateProjection(int ellapsedTimeForMove)
{
    if (updateFovy(ellapsedTimeForMove))
        updateProjectionMatrix();
}

void SceneController::prerenderScene()
{
    sortRenderOrder();
    m_engine->setViewport(m_width, m_height);
    m_engine->prerender(m_objects, m_order, m_maxModel);
}

void SceneController::renderScene()
{
    if (!isViewMoving())
        m_viewMoveTime = -1;
    m_engine->render(m_objects, m_order, m_maxModel, m_stage);
}

void SceneController::updateRotationFromAngle()
{
    m_rot = btQuaternion(btVector3(0.0f, 0.0f, 1.0f), MMDAIMathRadian(m_angle.z()))
            * btQuaternion(btVector3(1.0f, 0.0f, 0.0f), MMDAIMathRadian(m_angle.x()))
            * btQuaternion(btVector3(0.0f, 1.0f, 0.0f), MMDAIMathRadian(m_angle.y()));
}

bool SceneController::updateDistance(int ellapsedTimeForMove)
{
    /* if no difference, return */
    if (m_currentDistance == m_distance)
        return false;

    if (m_viewMoveTime == 0 || m_viewControlledByMotion) {
        /* immediately apply the target */
        m_currentDistance = m_distance;
    }
    else if (m_viewMoveTime > 0) {
        /* constant move */
        if (ellapsedTimeForMove >= m_viewMoveTime) {
            m_currentDistance = m_distance;
        }
        else {
            m_currentDistance = m_viewMoveStartDistance + (m_distance - m_viewMoveStartDistance) * ellapsedTimeForMove / m_viewMoveTime;
        }
    }
    else {
        float diff = fabs(m_currentDistance - m_distance);
        if (diff < RENDER_MINDISTANCEDIFF) {
            m_currentDistance = m_distance;
        }
        else {
            m_currentDistance = m_currentDistance * (RENDER_DISTANCESPEEDRATE) + m_distance * (1.0f - RENDER_DISTANCESPEEDRATE);
        }
    }
    return true;
}

bool SceneController::updateFovy(int ellapsedTimeForMove)
{
    /* if no difference, return */
    if (m_currentFovy == m_fovy)
        return false;
    if (m_viewMoveTime == 0 || m_viewControlledByMotion) {
        /* immediately apply the target */
        m_currentFovy = m_fovy;
    }
    else if (m_viewMoveTime > 0) {
        /* constant move */
        if (ellapsedTimeForMove >= m_viewMoveTime) {
            m_currentFovy = m_fovy;
        }
        else {
            m_currentFovy = m_viewMoveStartFovy + (m_fovy - m_viewMoveStartFovy) * ellapsedTimeForMove / m_viewMoveTime;
        }
    }
    else {
        float diff = fabs(m_currentFovy - m_fovy);
        if (diff < RENDER_MINFOVYDIFF) {
            m_currentFovy = m_fovy;
        }
        else {
            m_currentFovy = m_currentFovy * (RENDER_FOVYSPEEDRATE) + m_fovy * (1.0f - RENDER_FOVYSPEEDRATE);
        }
    }
    return true;
}

static void GLPerspective(float result[16], float fovy, float aspect, float znear, float zfar)
{
    const float f = 1.0f / tanf(fovy / 2.0f);
    const float a = f / aspect;
    const float b = f;
    const float c = (zfar + znear) / (znear - zfar);
    const float d = (2.0f * zfar * znear) / (znear - zfar);
    const float matrix[16] = {
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, c, -1,
        0, 0, d, 0
    };
    memcpy(result, matrix, sizeof(matrix));
}

void SceneController::updateModelViewMatrix()
{
    m_transMatrix.setIdentity();
    m_transMatrix.setRotation(m_currentRot);
    m_transMatrix.setOrigin(m_transMatrix * (-m_currentTrans) - btVector3(0.0f, 0.0f, m_currentDistance));
    m_engine->setModelView(m_transMatrix);
}

void SceneController::updateProjectionMatrix()
{
    float aspect = (m_width == 0) ? 1.0f : static_cast<float>(m_width) / m_height;
    float projection[16];
    //GLPerspective(projection, m_currentFovy, aspect, kRenderViewPointFrustumNear, kRenderViewPointFrustumFar);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(m_currentFovy, aspect, kRenderViewPointFrustumNear, kRenderViewPointFrustumFar);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glMatrixMode(GL_MODELVIEW);
    m_engine->setProjection(projection);
}

void SceneController::sortRenderOrder()
{
    if (m_numModel == 0)
        return;

    int size = 0;
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *object = m_objects[i];
        if (!object->isEnable() || !object->allowMotionFileDrop())
            continue;
        btVector3 pos = object->getModel()->getCenterBone()->getTransform().getOrigin();
        RenderDepth *depth = &m_depth[size];
        pos = m_transMatrix * pos;
        depth->distance = pos.z();
        depth->id = i;
        size++;
    }
    qsort(m_depth, size, sizeof(RenderDepth), compareDepth);
    for (int i = 0; i < size; i++)
        m_order[i] = m_depth[i].id;
    for (int i = 0; i < m_maxModel; i++) {
        PMDObject *object = m_objects[i];
        if (!object->isEnable() || !object->allowMotionFileDrop())
            m_order[size++] = i;
    }
}

} /* namespace */
