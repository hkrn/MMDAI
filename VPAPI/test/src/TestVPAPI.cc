/**

 Copyright (c) 2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "TestVPAPI.h"

#include "BaseKeyframeRefObject.h"
#include "BaseMotionTrack.h"
#include "CameraKeyframeRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "ModelProxy.h"
#include "MotionProxy.h"

#include <vpvl2/vpvl2.h>
#include <QtCore>
#include <QUndoGroup>
#include <QUndoStack>

using namespace vpvl2;

void TestVPAPI::project_initialize()
{
    ProjectProxy project;
    project.initializeOnce();
    QVERIFY(!project.isDirty());
    QVERIFY(project.motionProxies().contains(project.camera()->motion()));
    QVERIFY(project.motionProxies().contains(project.light()->motion()));
    QCOMPARE(project.camera()->motion()->parentProject(), &project);
    QCOMPARE(project.camera()->motion()->parentModel(), static_cast<ModelProxy *>(0));
    QCOMPARE(project.light()->motion()->parentProject(), &project);
    QCOMPARE(project.light()->motion()->parentModel(), static_cast<ModelProxy *>(0));
    QCOMPARE(project.camera()->track()->keyframes().size(), 2);
    QCOMPARE(project.light()->track()->keyframes().size(), 2);
}

void TestVPAPI::project_createModelProxy_data()
{
    QTest::addColumn<ProjectProxy::LanguageType>("language");
    QTest::newRow("DefaultLanguage") << ProjectProxy::DefaultLauguage;
    QTest::newRow("Japanese") << ProjectProxy::Japanese;
    QTest::newRow("English") << ProjectProxy::English;
}

void TestVPAPI::project_createModelProxy()
{
    QFETCH(ProjectProxy::LanguageType, language);
    ProjectProxy project;
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(IModel::kPMXModel));
    const QUuid &uuid = QUuid::createUuid();
    ModelProxy *modelProxy = project.createModelProxy(model.take(), uuid, QUrl());
    QCOMPARE(modelProxy->uuid(), uuid);
    QVERIFY(project.undoGroup()->stacks().contains(modelProxy->undoStack()));
    project.setLanguage(language);
    QCOMPARE(modelProxy->language(), language);
}

void TestVPAPI::project_createMotionProxy()
{
    ProjectProxy project;
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(IModel::kPMXModel));
    QScopedPointer<IMotion> motion(project.factoryInstanceRef()->newMotion(IMotion::kVMDMotion, model.data()));
    const QUuid &uuid = QUuid::createUuid();
    MotionProxy *motionProxy = project.createMotionProxy(motion.take(), uuid, QUrl());
    QCOMPARE(motionProxy->parentProject(), &project);
    QCOMPARE(project.findMotion(uuid), motionProxy);
    QCOMPARE(project.resolveMotionProxy(motionProxy->data()), motionProxy);
    QCOMPARE(motionProxy->uuid(), uuid);
    QVERIFY(project.motionProxies().contains(motionProxy));
    QVERIFY(project.projectInstanceRef()->containsMotion(motionProxy->data()));
    QVERIFY(project.undoGroup()->stacks().contains(motionProxy->undoStack()));
    QCOMPARE(project.createMotionProxy(motionProxy->data(), uuid, QUrl()), static_cast<MotionProxy *>(0));
}

void TestVPAPI::project_addModelProxy()
{
    ProjectProxy project;
    QSignalSpy modelDidAdd(&project, SIGNAL(modelDidAdd(ModelProxy*,bool)));
    QSignalSpy motionDidLoad(&project, SIGNAL(motionDidLoad(MotionProxy*)));
    QSignalSpy modelDidCommitUploading(&project, SIGNAL(modelDidCommitUploading()));
    QSignalSpy availableModelsChanged(&project, SIGNAL(availableModelsChanged()));
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(IModel::kPMXModel));
    const QUuid &uuid = QUuid::createUuid();
    ModelProxy *modelProxy = project.createModelProxy(model.take(), uuid, QUrl());
    QSignalSpy childMotionChanged(modelProxy, SIGNAL(childMotionChanged()));
    project.addModel(modelProxy);
    QCOMPARE(modelDidAdd.size(), 1);
    QCOMPARE(motionDidLoad.size(), 1);
    QCOMPARE(modelDidCommitUploading.size() ,1);
    QCOMPARE(availableModelsChanged.size(), 1);
    QCOMPARE(childMotionChanged.size(), 1);
    QVERIFY(project.isDirty());
    QCOMPARE(modelProxy->parentProject(), &project);
    QCOMPARE(project.findModel(uuid), modelProxy);
    QCOMPARE(project.resolveModelProxy(modelProxy->data()), modelProxy);
    QVERIFY(project.modelProxies().contains(modelProxy));
    QCOMPARE(project.currentModel(), modelProxy);
    QVERIFY(project.motionProxies().contains(modelProxy->childMotion()));
    QCOMPARE(modelProxy->childMotion()->parentProject(), &project);
    QCOMPARE(modelProxy->childMotion()->parentModel(), modelProxy);
}

void TestVPAPI::project_initializeMotion()
{
    ProjectProxy project;
    QSignalSpy motionDidInitialize(&project, SIGNAL(motionDidInitialize()));
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(IModel::kPMXModel));
    const QUuid &uuid = QUuid::createUuid();
    ModelProxy *modelProxy = project.createModelProxy(model.take(), uuid, QUrl());
    QSignalSpy childMotionChanged(modelProxy, SIGNAL(childMotionChanged()));
    project.addModel(modelProxy);
    QUuid oldUUID = modelProxy->childMotion()->uuid();
    project.initializeMotion(modelProxy, ProjectProxy::ModelMotion);
    QVERIFY(modelProxy->childMotion()->uuid() != oldUUID);
    QCOMPARE(childMotionChanged.size(), 3);
    QCOMPARE(motionDidInitialize.size(), 1);
}

void TestVPAPI::project_seek()
{
    ProjectProxy project;
    QSignalSpy currrentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QSignalSpy cameraDidReset(project.camera(), SIGNAL(cameraDidReset()));
    QSignalSpy lightDidReset(project.light(), SIGNAL(lightDidReset()));
    project.setCurrentTimeIndex(42);
    project.setCurrentTimeIndex(42);
    QCOMPARE(project.currentTimeIndex(), qreal(42));
    QCOMPARE(project.projectInstanceRef()->currentTimeIndex(), IKeyframe::TimeIndex(42));
    QCOMPARE(currrentTimeIndexChanged.size(), 1);
    QCOMPARE(cameraDidReset.size(), 1);
    QCOMPARE(lightDidReset.size(), 1);
}

void TestVPAPI::project_rewind()
{
    ProjectProxy project;
    QSignalSpy currrentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QSignalSpy cameraDidReset(project.camera(), SIGNAL(cameraDidReset()));
    QSignalSpy lightDidReset(project.light(), SIGNAL(lightDidReset()));
    project.rewind();
    project.rewind();
    QCOMPARE(project.currentTimeIndex(), qreal(0));
    QCOMPARE(project.projectInstanceRef()->currentTimeIndex(), IKeyframe::TimeIndex(0));
    QCOMPARE(currrentTimeIndexChanged.size(), 2);
    QCOMPARE(cameraDidReset.size(), 2);
    QCOMPARE(lightDidReset.size(), 2);
}

void TestVPAPI::project_reset()
{
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(IModel::kPMXModel));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    project.addModel(modelProxy);
    project.setCurrentTimeIndex(42);
    project.reset();
    QCOMPARE(project.currentTimeIndex(), qreal(0));
    QCOMPARE(project.currentModel(), static_cast<ModelProxy *>(0));
    QCOMPARE(project.currentMotion(), static_cast<MotionProxy *>(0));
}

void TestVPAPI::project_deleteModel()
{
    ProjectProxy project;
    QSignalSpy modelWillRemove(&project, SIGNAL(modelWillRemove(ModelProxy*)));
    QSignalSpy modelDidRemove(&project, SIGNAL(modelDidRemove(ModelProxy*)));
    QSignalSpy modelDidCommitDeleting(&project, SIGNAL(modelDidCommitDeleting()));
    QSignalSpy availableModelsChanged(&project, SIGNAL(availableModelsChanged()));
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(IModel::kPMXModel));
    const QUuid &uuid = QUuid::createUuid();
    ModelProxy *modelProxy = project.createModelProxy(model.take(), uuid, QUrl());
    project.addModel(modelProxy);
    modelProxy->undoStack()->push(new QUndoCommand());
    /* modelProxy will be removed but not deleted */
    project.deleteModel(modelProxy);
    QVERIFY(project.isDirty());
    QCOMPARE(project.findModel(uuid), static_cast<ModelProxy *>(0));
    QCOMPARE(project.resolveModelProxy(modelProxy->data()), static_cast<ModelProxy *>(0));
    QVERIFY(!project.modelProxies().contains(modelProxy));
    QCOMPARE(project.currentModel(), static_cast<ModelProxy *>(0));
    QVERIFY(!project.motionProxies().contains(modelProxy->childMotion()));
    QVERIFY(!project.projectInstanceRef()->containsModel(modelProxy->data()));
    QVERIFY(!project.undoGroup()->stacks().contains(modelProxy->undoStack()));
    QVERIFY(modelProxy->undoStack()->isClean());
    QVERIFY(!project.canUndo());
    QVERIFY(!project.canRedo());
    QCOMPARE(modelWillRemove.size(), 1);
    QCOMPARE(modelDidRemove.size(), 1);
    QCOMPARE(modelDidCommitDeleting.size(), 1);
    QCOMPARE(availableModelsChanged.size(), 2);
}

void TestVPAPI::project_deleteMotion()
{
    ProjectProxy project;
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(IModel::kPMXModel));
    QScopedPointer<IMotion> motion(project.factoryInstanceRef()->newMotion(IMotion::kVMDMotion, model.data()));
    const QUuid &uuid = QUuid::createUuid();
    MotionProxy *motionProxy = project.createMotionProxy(motion.take(), uuid, QUrl());
    motionProxy->undoStack()->push(new QUndoCommand());
    project.deleteMotion(motionProxy, false);
    QCOMPARE(project.findMotion(uuid), static_cast<MotionProxy *>(0));
    QVERIFY(project.motionProxies().isEmpty());
    QCOMPARE(project.currentMotion(), static_cast<MotionProxy *>(0));
    QVERIFY(project.projectInstanceRef()->motionUUIDs().empty());
}

void TestVPAPI::motion_addAndRemoveCameraKeyframe()
{
    ProjectProxy project;
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    project.setCurrentMotion(project.camera()->motion());
    addKeyframe(project, project.camera()->track(), project.camera(), undoDidPerform, redoDidPerform, currentTimeIndexChanged);
    removeKeyframe(project, project.camera()->track(), undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_addAndUpdateCameraKeyframe()
{
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    project.initializeOnce();
    project.setCurrentMotion(project.camera()->motion());
    project.camera()->setAngle(QVector3D(1, 2, 3));
    project.camera()->setDistance(42);
    project.camera()->setFov(84);
    project.camera()->setLookAt(QVector3D(4, 5, 6));
    project.camera()->motion()->updateKeyframe(project.camera(), 0);
    QCOMPARE(project.camera()->angle(), QVector3D(1, 2, 3));
    QCOMPARE(project.camera()->distance(), qreal(42));
    QCOMPARE(project.camera()->fov(), qreal(84));
    QCOMPARE(project.camera()->lookAt(), QVector3D(4, 5, 6));
    QCOMPARE(project.camera()->track()->keyframes().size(), 2);
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(project.camera()->track()->keyframes().size(), 2);
    QCOMPARE(project.camera()->angle(), QVector3D());
    QCOMPARE(project.camera()->distance(), qreal(50));
    QCOMPARE(project.camera()->fov(), qreal(27));
    QCOMPARE(project.camera()->lookAt(), QVector3D(0, 10, 0));
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QCOMPARE(project.camera()->track()->keyframes().size(), 2);
    QCOMPARE(project.camera()->angle(), QVector3D(1, 2, 3));
    QCOMPARE(project.camera()->distance(), qreal(42));
    QCOMPARE(project.camera()->fov(), qreal(84));
    QCOMPARE(project.camera()->lookAt(), QVector3D(4, 5, 6));
}

void TestVPAPI::motion_addAndRemoveLightKeyframe()
{
    ProjectProxy project;
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    project.setCurrentMotion(project.light()->motion());
    addKeyframe(project, project.light()->track(), project.light(), undoDidPerform, redoDidPerform, currentTimeIndexChanged);
    removeKeyframe(project, project.light()->track(), undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_addAndUpdateLightKeyframe()
{
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    project.initializeOnce();
    project.setCurrentMotion(project.light()->motion());
    project.light()->setColor(Qt::red);
    project.light()->setDirection(QVector3D(1, 2, 3));
    project.light()->motion()->updateKeyframe(project.light(), 0);
    QCOMPARE(project.light()->color(), QColor(Qt::red));
    QCOMPARE(project.light()->direction(), QVector3D(1, 2, 3));
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(project.light()->color(), QColor::fromRgbF(0.6, 0.6, 0.6));
    QCOMPARE(project.light()->direction(), QVector3D(-0.5, -1, -0.5));
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QCOMPARE(project.light()->color(), QColor(Qt::red));
    QCOMPARE(project.light()->direction(), QVector3D(1, 2, 3));
}

void TestVPAPI::addKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *object, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged)
{
    track->parentMotion()->addKeyframe(object, 42);
    BaseKeyframeRefObject *keyframeRef = track->findKeyframeByTimeIndex(42);
    QVERIFY(keyframeRef);
    QCOMPARE(keyframeRef->timeIndex(), quint64(42));
    QCOMPARE(track->keyframes().size(), 3);
    QCOMPARE(track->findKeyframeByTimeIndex(42), keyframeRef);
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(currentTimeIndexChanged.size(), 1);
    QCOMPARE(track->keyframes().size(), 2);
    QVERIFY(!track->findKeyframeByTimeIndex(42));
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QCOMPARE(currentTimeIndexChanged.size(), 2);
    QCOMPARE(track->keyframes().size(), 3);
    QCOMPARE(track->findKeyframeByTimeIndex(42), keyframeRef);
}

void TestVPAPI::removeKeyframe(ProjectProxy &project, BaseMotionTrack *track, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged)
{
    BaseKeyframeRefObject *keyframe = track->findKeyframeByTimeIndex(42);
    track->parentMotion()->removeKeyframe(keyframe);
    QCOMPARE(track->keyframes().size(), 2);
    QVERIFY(!track->findKeyframeByTimeIndex(42));
    project.undo();
    QCOMPARE(undoDidPerform.size(), 2);
    QCOMPARE(currentTimeIndexChanged.size(), 3);
    QCOMPARE(track->keyframes().size(), 3);
    QCOMPARE(track->findKeyframeByTimeIndex(42), keyframe);
    project.redo();
    QCOMPARE(redoDidPerform.size(), 2);
    QCOMPARE(currentTimeIndexChanged.size(), 4);
    QCOMPARE(track->keyframes().size(), 2);
    QVERIFY(!track->findKeyframeByTimeIndex(42));
}

QTEST_MAIN(TestVPAPI)
