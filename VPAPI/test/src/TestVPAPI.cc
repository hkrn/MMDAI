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
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "CameraKeyframeRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "JointRefObject.h"
#include "LabelRefObject.h"
#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "MaterialRefObject.h"
#include "ModelProxy.h"
#include "MorphMotionTrack.h"
#include "MorphRefObject.h"
#include "MotionProxy.h"
#include "RigidBodyRefObject.h"
#include "VertexRefObject.h"
#include "WorldProxy.h"

#include <vpvl2/vpvl2.h>
#include <QtCore>
#include <QUndoGroup>
#include <QUndoStack>

using namespace vpvl2;

namespace {

const QString kBoneName = QStringLiteral("testBone");
const QString kMorphName = QStringLiteral("testMorph");
const quint64 kTimeIndex = 42;
const qreal kWeight = 0.42;

}

void TestVPAPI::initTestCase()
{
    qRegisterMetaType<ModelProxy *>("ModelProxy");
    qRegisterMetaType<MotionProxy *>("MotionProxy");
}

void TestVPAPI::project_initialize()
{
    ProjectProxy project;
    project.initializeOnce();
    QVERIFY(!project.isDirty());
    QVERIFY(project.modelProxies().isEmpty());
    QCOMPARE(project.motionProxies().size(), 2);
    QVERIFY(project.motionProxies().contains(project.camera()->motion()));
    QVERIFY(project.motionProxies().contains(project.light()->motion()));
    QCOMPARE(project.camera()->motion()->parentProject(), &project);
    QCOMPARE(project.camera()->motion()->parentModel(), static_cast<ModelProxy *>(0));
    QCOMPARE(project.light()->motion()->parentProject(), &project);
    QCOMPARE(project.light()->motion()->parentModel(), static_cast<ModelProxy *>(0));
    QCOMPARE(project.camera()->track()->keyframes().size(), 2);
    QCOMPARE(project.light()->track()->keyframes().size(), 2);
}

void TestVPAPI::project_create_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::project_create()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    QSignalSpy projectWillCreate(&project, SIGNAL(projectWillCreate()));
    QSignalSpy projectDidCreate(&project, SIGNAL(projectDidCreate()));
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    project.addModel(project.createModelProxy(model.take(), QUuid::createUuid(), QUrl()));
    project.setTitle(QStringLiteral("hogehoge"));
    project.setAudioSource(QUrl("http://localhost"));
    project.setVideoSource(QUrl("http://localhost"));
    project.setAccelerationType(ProjectProxy::NoAcceleration);
    project.setLanguage(ProjectProxy::English);
    project.setScreenColor(Qt::red);
    project.setGridVisible(false);
    project.setLoop(true);
    project.setScreenColor(Qt::red);
    project.createAsync();
    project.enqueuedModelsDidDelete();
    qApp->processEvents();
    QVERIFY(!project.isDirty());
    QVERIFY(project.modelProxies().isEmpty());
    QCOMPARE(project.motionProxies().size(), 2);
    QVERIFY(project.motionProxies().contains(project.camera()->motion()));
    QVERIFY(project.motionProxies().contains(project.light()->motion()));
    QCOMPARE(projectWillCreate.size(), 1);
    QCOMPARE(projectDidCreate.size(), 1);
    QCOMPARE(project.title(), tr("Untitled Project"));
    QVERIFY(project.audioSource().isEmpty());
    QVERIFY(project.videoSource().isEmpty());
    QCOMPARE(project.accelerationType(), ProjectProxy::ParallelAcceleration);
    QCOMPARE(project.language(), ProjectProxy::DefaultLauguage);
    QCOMPARE(project.screenColor(), QColor(Qt::white));
    QVERIFY(project.isGridVisible());
    QVERIFY(!project.isLoop());
    QCOMPARE(project.screenColor(), QColor(Qt::white));
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

void TestVPAPI::project_createMotionProxy_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::addColumn<IMotion::Type>("motionType");
    QTest::newRow("PMD+VMD") << IModel::kPMDModel << IMotion::kVMDMotion;
    QTest::newRow("PMX+VMD") << IModel::kPMXModel << IMotion::kVMDMotion;
    QTest::newRow("PMD+MVD") << IModel::kPMDModel << IMotion::kMVDMotion;
    QTest::newRow("PMX+MVD") << IModel::kPMXModel << IMotion::kMVDMotion;
}

void TestVPAPI::project_createMotionProxy()
{
    QFETCH(IModel::Type, modelType);
    QFETCH(IMotion::Type, motionType);
    ProjectProxy project;
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    QScopedPointer<IMotion> motion(project.factoryInstanceRef()->newMotion(motionType, model.data()));
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

void TestVPAPI::project_addModelProxy_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::project_addModelProxy()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    QSignalSpy modelDidAdd(&project, SIGNAL(modelDidAdd(ModelProxy*,bool)));
    QSignalSpy motionDidLoad(&project, SIGNAL(motionDidLoad(MotionProxy*)));
    QSignalSpy modelDidCommitUploading(&project, SIGNAL(modelDidCommitUploading()));
    QSignalSpy availableModelsChanged(&project, SIGNAL(availableModelsChanged()));
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
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

void TestVPAPI::project_initializeMotion_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::project_initializeMotion()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    QSignalSpy motionDidInitialize(&project, SIGNAL(motionDidInitialize()));
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
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
    project.setCurrentTimeIndex(kTimeIndex);
    project.setCurrentTimeIndex(kTimeIndex);
    QCOMPARE(project.currentTimeIndex(), qreal(kTimeIndex));
    QCOMPARE(project.projectInstanceRef()->currentTimeIndex(), IKeyframe::TimeIndex(kTimeIndex));
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

void TestVPAPI::project_reset_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::project_reset()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    project.addModel(modelProxy);
    project.setCurrentTimeIndex(kTimeIndex);
    project.reset();
    QCOMPARE(project.currentTimeIndex(), qreal(0));
    QCOMPARE(project.currentModel(), static_cast<ModelProxy *>(0));
    QCOMPARE(project.currentMotion(), static_cast<MotionProxy *>(0));
}

void TestVPAPI::project_deleteModel_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::project_deleteModel()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    QSignalSpy modelWillRemove(&project, SIGNAL(modelWillRemove(ModelProxy*)));
    QSignalSpy modelDidRemove(&project, SIGNAL(modelDidRemove(ModelProxy*)));
    QSignalSpy modelDidCommitDeleting(&project, SIGNAL(modelDidCommitDeleting()));
    QSignalSpy availableModelsChanged(&project, SIGNAL(availableModelsChanged()));
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
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

void TestVPAPI::project_deleteMotion_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::addColumn<IMotion::Type>("motionType");
    QTest::newRow("PMD+VMD") << IModel::kPMDModel << IMotion::kVMDMotion;
    QTest::newRow("PMX+VMD") << IModel::kPMXModel << IMotion::kVMDMotion;
    QTest::newRow("PMD+MVD") << IModel::kPMDModel << IMotion::kMVDMotion;
    QTest::newRow("PMX+MVD") << IModel::kPMXModel << IMotion::kMVDMotion;
}

void TestVPAPI::project_deleteMotion()
{
    QFETCH(IModel::Type, modelType);
    QFETCH(IMotion::Type, motionType);
    ProjectProxy project;
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    QScopedPointer<IMotion> motion(project.factoryInstanceRef()->newMotion(motionType, model.data()));
    const QUuid &uuid = QUuid::createUuid();
    MotionProxy *motionProxy = project.createMotionProxy(motion.take(), uuid, QUrl());
    motionProxy->undoStack()->push(new QUndoCommand());
    project.deleteMotion(motionProxy, false);
    QCOMPARE(project.findMotion(uuid), static_cast<MotionProxy *>(0));
    QVERIFY(project.motionProxies().isEmpty());
    QCOMPARE(project.currentMotion(), static_cast<MotionProxy *>(0));
    QVERIFY(project.projectInstanceRef()->motionUUIDs().empty());
}

void TestVPAPI::model_addAndRemoveVertex_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_addAndRemoveVertex()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy allVerticesChanged(modelProxy, SIGNAL(allVerticesChanged()));
    VertexRefObject *vertex = modelProxy->createVertex();
    QCOMPARE(vertex->parentModel(), modelProxy);
    QCOMPARE(modelProxy->findVertexByUuid(vertex->uuid()), vertex);
    QVERIFY(modelProxy->allVertexRefs().contains(vertex));
    QCOMPARE(modelProxy->data()->count(IModel::kVertex), 1);
    QVERIFY(modelProxy->removeVertex(vertex));
    QVERIFY(!modelProxy->removeVertex(vertex));
    QCOMPARE(modelProxy->findVertexByUuid(vertex->uuid()), static_cast<VertexRefObject *>(0));
    QVERIFY(!modelProxy->allVertexRefs().contains(vertex));
    QCOMPARE(modelProxy->data()->count(IModel::kVertex), 0);
    QCOMPARE(allVerticesChanged.size(), 2);
    delete vertex;
}

void TestVPAPI::model_addAndRemoveMaterial_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_addAndRemoveMaterial()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy allMaterialsChanged(modelProxy, SIGNAL(allMaterialsChanged()));
    MaterialRefObject *material = modelProxy->createMaterial();
    QCOMPARE(material->parentModel(), modelProxy);
    QCOMPARE(modelProxy->findMaterialByUuid(material->uuid()), material);
    QVERIFY(modelProxy->allMaterialRefs().contains(material));
    QCOMPARE(modelProxy->data()->count(IModel::kMaterial), 1);
    QVERIFY(modelProxy->removeMaterial(material));
    QVERIFY(!modelProxy->removeMaterial(material));
    QCOMPARE(modelProxy->findMaterialByUuid(material->uuid()), static_cast<MaterialRefObject *>(0));
    QVERIFY(!modelProxy->allMaterialRefs().contains(material));
    QCOMPARE(modelProxy->data()->count(IModel::kMaterial), 0);
    QCOMPARE(allMaterialsChanged.size(), 2);
    delete material;
}

void TestVPAPI::model_addAndRemoveBone_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_addAndRemoveBone()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy allBonesChanged(modelProxy, SIGNAL(allBonesChanged()));
    BoneRefObject *bone = modelProxy->createBone();
    QCOMPARE(bone->parentModel(), modelProxy);
    QCOMPARE(modelProxy->findBoneByUuid(bone->uuid()), bone);
    QVERIFY(modelProxy->allBoneRefs().contains(bone));
    QCOMPARE(modelProxy->data()->count(IModel::kBone), 1);
    QVERIFY(modelProxy->removeBone(bone));
    QVERIFY(!modelProxy->removeBone(bone));
    QCOMPARE(modelProxy->findBoneByUuid(bone->uuid()), static_cast<BoneRefObject *>(0));
    QVERIFY(!modelProxy->allBoneRefs().contains(bone));
    QCOMPARE(modelProxy->data()->count(IModel::kBone), 0);
    QCOMPARE(allBonesChanged.size(), 2);
    delete bone;
}

void TestVPAPI::model_addAndRemoveMorph_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_addAndRemoveMorph()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy allMorphsChanged(modelProxy, SIGNAL(allMorphsChanged()));
    MorphRefObject *morph = modelProxy->createMorph();
    QCOMPARE(morph->parentModel(), modelProxy);
    QCOMPARE(modelProxy->findMorphByUuid(morph->uuid()), morph);
    QVERIFY(modelProxy->allMorphRefs().contains(morph));
    QCOMPARE(modelProxy->data()->count(IModel::kMorph), 1);
    QVERIFY(modelProxy->removeMorph(morph));
    QVERIFY(!modelProxy->removeMorph(morph));
    QCOMPARE(modelProxy->findMorphByUuid(morph->uuid()), static_cast<MorphRefObject *>(0));
    QVERIFY(!modelProxy->allMorphRefs().contains(morph));
    QCOMPARE(modelProxy->data()->count(IModel::kMorph), 0);
    QCOMPARE(allMorphsChanged.size(), 2);
    delete morph;
}

void TestVPAPI::model_addAndRemoveLabel_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_addAndRemoveLabel()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy allLabelsChanged(modelProxy, SIGNAL(allLabelsChanged()));
    LabelRefObject *label = modelProxy->createLabel();
    QCOMPARE(label->parentModel(), modelProxy);
    QVERIFY(modelProxy->allLabelRefs().contains(label));
    QVERIFY(modelProxy->removeLabel(label));
    QVERIFY(!modelProxy->removeLabel(label));
    QVERIFY(!modelProxy->allLabelRefs().contains(label));
    QCOMPARE(allLabelsChanged.size(), 2);
    delete label;
}

void TestVPAPI::model_addAndRemoveRigidBody_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_addAndRemoveRigidBody()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy allRigidBodiesChanged(modelProxy, SIGNAL(allRigidBodiesChanged()));
    RigidBodyRefObject *rigidBody = modelProxy->createRigidBody();
    QCOMPARE(rigidBody->parentModel(), modelProxy);
    QCOMPARE(modelProxy->findRigidBodyByUuid(rigidBody->uuid()), rigidBody);
    QVERIFY(modelProxy->allRigidBodyRefs().contains(rigidBody));
    QCOMPARE(modelProxy->data()->count(IModel::kRigidBody), 1);
    QVERIFY(modelProxy->removeRigidBody(rigidBody));
    QVERIFY(!modelProxy->removeRigidBody(rigidBody));
    QCOMPARE(modelProxy->findMaterialByUuid(rigidBody->uuid()), static_cast<MaterialRefObject *>(0));
    QVERIFY(!modelProxy->allRigidBodyRefs().contains(rigidBody));
    QCOMPARE(modelProxy->data()->count(IModel::kRigidBody), 0);
    QCOMPARE(allRigidBodiesChanged.size(), 2);
    delete rigidBody;
}

void TestVPAPI::model_addAndRemoveJoint_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_addAndRemoveJoint()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy allJointsChanged(modelProxy, SIGNAL(allJointsChanged()));
    JointRefObject *joint = modelProxy->createJoint();
    QCOMPARE(joint->parentModel(), modelProxy);
    QCOMPARE(modelProxy->findJointByUuid(joint->uuid()), joint);
    QVERIFY(modelProxy->allJointRefs().contains(joint));
    QCOMPARE(modelProxy->data()->count(IModel::kJoint), 1);
    QVERIFY(modelProxy->removeJoint(joint));
    QVERIFY(!modelProxy->removeJoint(joint));
    QCOMPARE(modelProxy->findJointByUuid(joint->uuid()), static_cast<JointRefObject *>(0));
    QVERIFY(!modelProxy->allJointRefs().contains(joint));
    QCOMPARE(modelProxy->data()->count(IModel::kJoint), 0);
    QCOMPARE(allJointsChanged.size(), 2);
    delete joint;
}

void TestVPAPI::model_translateTransform_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_translateTransform()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy transformDidBegin(modelProxy, SIGNAL(transformDidBegin()));
    QSignalSpy targetBonesDidTranslate(modelProxy, SIGNAL(targetBonesDidTranslate()));
    QSignalSpy transformDidCommit(modelProxy, SIGNAL(transformDidCommit()));
    QSignalSpy transformDidDiscard(modelProxy, SIGNAL(transformDidDiscard()));
    modelProxy->beginTransform(0);
    QVERIFY(modelProxy->isMoving());
    modelProxy->translate(1);
    modelProxy->commitTransform();
    QVERIFY(!modelProxy->isMoving());
    modelProxy->beginTransform(0);
    QVERIFY(modelProxy->isMoving());
    modelProxy->translate(1);
    modelProxy->discardTransform();
    QVERIFY(!modelProxy->isMoving());
    QCOMPARE(transformDidBegin.size(), 2);
    QCOMPARE(targetBonesDidTranslate.size(), 2);
    QCOMPARE(transformDidCommit.size(), 1);
    QCOMPARE(transformDidDiscard.size(), 1);
}

void TestVPAPI::model_rotateTransform_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_rotateTransform()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    QSignalSpy transformDidBegin(modelProxy, SIGNAL(transformDidBegin()));
    QSignalSpy targetBonesDidRotate(modelProxy, SIGNAL(targetBonesDidRotate()));
    QSignalSpy transformDidCommit(modelProxy, SIGNAL(transformDidCommit()));
    QSignalSpy transformDidDiscard(modelProxy, SIGNAL(transformDidDiscard()));
    modelProxy->beginTransform(0);
    QVERIFY(modelProxy->isMoving());
    modelProxy->rotate(1);
    modelProxy->commitTransform();
    QVERIFY(!modelProxy->isMoving());
    modelProxy->beginTransform(0);
    QVERIFY(modelProxy->isMoving());
    modelProxy->rotate(1);
    modelProxy->discardTransform();
    QVERIFY(!modelProxy->isMoving());
    QCOMPARE(transformDidBegin.size(), 2);
    QCOMPARE(targetBonesDidRotate.size(), 2);
    QCOMPARE(transformDidCommit.size(), 1);
    QCOMPARE(transformDidDiscard.size(), 1);
}

void TestVPAPI::model_release_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::model_release()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    project.addModel(modelProxy);
    modelProxy->release();
    model.reset(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy2 = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    modelProxy2->release();
    qApp->processEvents();
}

void TestVPAPI::motion_addAndRemoveCameraKeyframe()
{
    ProjectProxy project;
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    project.setCurrentMotion(project.camera()->motion());
    testAddKeyframe(project, project.camera()->track(), project.camera(), 2, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
    testRemoveKeyframe(project, project.camera()->track(), project.camera(), 3, 2, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_addAndUpdateCameraKeyframe()
{
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    project.initializeOnce();
    CameraRefObject *camera = project.camera();
    project.setCurrentMotion(camera->motion());
    camera->setAngle(QVector3D(1, 2, 3));
    camera->setDistance(kTimeIndex);
    camera->setFov(84);
    camera->setLookAt(QVector3D(4, 5, 6));
    camera->motion()->updateKeyframe(camera, 0);
    QCOMPARE(camera->angle(), QVector3D(1, 2, 3));
    QCOMPARE(camera->distance(), qreal(kTimeIndex));
    QCOMPARE(camera->fov(), qreal(84));
    QCOMPARE(camera->lookAt(), QVector3D(4, 5, 6));
    QCOMPARE(camera->track()->keyframes().size(), 2);
    QCOMPARE(camera->motion()->data()->countKeyframes(camera->track()->type()), 2);
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(camera->track()->keyframes().size(), 2);
    QCOMPARE(camera->angle(), QVector3D());
    QCOMPARE(camera->distance(), qreal(50));
    QCOMPARE(camera->fov(), qreal(27));
    QCOMPARE(camera->lookAt(), QVector3D(0, 10, 0));
    QCOMPARE(camera->motion()->data()->countKeyframes(camera->track()->type()), 2);
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QCOMPARE(camera->track()->keyframes().size(), 2);
    QCOMPARE(camera->angle(), QVector3D(1, 2, 3));
    QCOMPARE(camera->distance(), qreal(kTimeIndex));
    QCOMPARE(camera->fov(), qreal(84));
    QCOMPARE(camera->lookAt(), QVector3D(4, 5, 6));
    QCOMPARE(camera->motion()->data()->countKeyframes(camera->track()->type()), 2);
}

void TestVPAPI::motion_addAndRemoveLightKeyframe()
{
    ProjectProxy project;
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    project.setCurrentMotion(project.light()->motion());
    testAddKeyframe(project, project.light()->track(), project.light(), 2, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
    testRemoveKeyframe(project, project.light()->track(), project.light(), 3, 2, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_addAndUpdateLightKeyframe()
{
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    project.initializeOnce();
    LightRefObject *light = project.light();
    project.setCurrentMotion(light->motion());
    light->setColor(Qt::red);
    light->setDirection(QVector3D(1, 2, 3));
    light->motion()->updateKeyframe(light, 0);
    QCOMPARE(light->color(), QColor(Qt::red));
    QCOMPARE(light->direction(), QVector3D(1, 2, 3));
    QCOMPARE(light->track()->keyframes().size(), 2);
    QCOMPARE(light->motion()->data()->countKeyframes(light->track()->type()), 2);
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(light->color(), QColor::fromRgbF(0.6, 0.6, 0.6));
    QCOMPARE(light->direction(), QVector3D(-0.5, -1, -0.5));
    QCOMPARE(light->track()->keyframes().size(), 2);
    QCOMPARE(light->motion()->data()->countKeyframes(light->track()->type()), 2);
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QCOMPARE(light->color(), QColor(Qt::red));
    QCOMPARE(light->direction(), QVector3D(1, 2, 3));
    QCOMPARE(light->track()->keyframes().size(), 2);
    QCOMPARE(light->motion()->data()->countKeyframes(light->track()->type()), 2);
}

void TestVPAPI::motion_addAndRemoveBoneKeyframe_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::motion_addAndRemoveBoneKeyframe()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    BoneRefObject *bone = modelProxy->createBone();
    bone->setName(kBoneName);
    project.addModel(modelProxy);
    project.initializeMotion(modelProxy, ProjectProxy::ModelMotion);
    BoneMotionTrack *track = modelProxy->childMotion()->findBoneMotionTrack(bone);
    testAddKeyframe(project, track, bone, 1, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
    testRemoveKeyframe(project, track, bone, 2, 2, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_addAndUpdateBoneKeyframe_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::motion_addAndUpdateBoneKeyframe()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    BoneRefObject *bone = modelProxy->createBone();
    bone->setName(kBoneName);
    modelProxy->initialize(true);
    project.addModel(modelProxy);
    project.initializeMotion(modelProxy, ProjectProxy::ModelMotion);
    MotionProxy *motionProxy = modelProxy->childMotion();
    BoneMotionTrack *track = motionProxy->findBoneMotionTrack(bone);
    bone->setLocalTranslation(QVector3D(1, 2, 3));
    bone->setLocalOrientation(QQuaternion(0.4, 0.1, 0.2, 0.3));
    motionProxy->updateKeyframe(bone, 0);
    QCOMPARE(bone->localTranslation(), QVector3D(1, 2, 3));
    QCOMPARE(bone->localOrientation(), QQuaternion(0.4, 0.1, 0.2, 0.3));
    QCOMPARE(track->keyframes().size(), 1);
    QCOMPARE(motionProxy->data()->countKeyframes(track->type()), 1);
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(bone->localTranslation(), QVector3D());
    QCOMPARE(bone->localOrientation(), QQuaternion());
    QCOMPARE(track->keyframes().size(), 1);
    QCOMPARE(motionProxy->data()->countKeyframes(track->type()), 1);
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QCOMPARE(bone->localTranslation(), QVector3D(1, 2, 3));
    QCOMPARE(bone->localOrientation(), QQuaternion(0.4, 0.1, 0.2, 0.3));
    QCOMPARE(track->keyframes().size(), 1);
    QCOMPARE(motionProxy->data()->countKeyframes(track->type()), 1);
}

void TestVPAPI::motion_addAndRemoveMorphKeyframe_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::motion_addAndRemoveMorphKeyframe()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    MorphRefObject *morph = modelProxy->createMorph();
    morph->setName(kMorphName);
    project.addModel(modelProxy);
    project.initializeMotion(modelProxy, ProjectProxy::ModelMotion);
    MorphMotionTrack *track = modelProxy->childMotion()->findMorphMotionTrack(morph);
    testAddKeyframe(project, track, morph, 1, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
    testRemoveKeyframe(project, track, morph, 2, 2, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_addAndUpdateMorphKeyframe_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::newRow("PMD") << IModel::kPMDModel;
    QTest::newRow("PMX") << IModel::kPMXModel;
}

void TestVPAPI::motion_addAndUpdateMorphKeyframe()
{
    QFETCH(IModel::Type, modelType);
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    MorphRefObject *morph = modelProxy->createMorph();
    morph->setName(kMorphName);
    modelProxy->initialize(true);
    project.addModel(modelProxy);
    project.initializeMotion(modelProxy, ProjectProxy::ModelMotion);
    MotionProxy *motionProxy = modelProxy->childMotion();
    MorphMotionTrack *track = motionProxy->findMorphMotionTrack(morph);
    morph->setWeight(kWeight);
    motionProxy->updateKeyframe(morph, 0);
    QVERIFY(qFuzzyCompare(morph->weight(), IMorph::WeightPrecision(kWeight)));
    QCOMPARE(track->keyframes().size(), 1);
    QCOMPARE(motionProxy->data()->countKeyframes(track->type()), 1);
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(morph->weight(), IMorph::WeightPrecision(0));
    QCOMPARE(track->keyframes().size(), 1);
    QCOMPARE(motionProxy->data()->countKeyframes(track->type()), 1);
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QVERIFY(qFuzzyCompare(morph->weight(), IMorph::WeightPrecision(kWeight)));
    QCOMPARE(track->keyframes().size(), 1);
    QCOMPARE(motionProxy->data()->countKeyframes(track->type()), 1);
}

void TestVPAPI::motion_copyAndPasteAndCutCameraKeyframe_data()
{
    QTest::addColumn<bool>("inversed");
    QTest::newRow("inversed=false") << false;
    QTest::newRow("inversed=true") << true;
}

void TestVPAPI::motion_copyAndPasteAndCutCameraKeyframe()
{
    QFETCH(bool, inversed);
    ProjectProxy project;
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QObject *object = project.camera();
    MotionProxy *parentMotion = project.camera()->motion();
    project.setCurrentMotion(parentMotion);
    BaseMotionTrack *track = project.camera()->track();
    testCopyAndPasteAndTest(project, track, object, inversed, 2, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_copyAndPasteAndCutLightKeyframe_data()
{
    QTest::addColumn<bool>("inversed");
    QTest::newRow("inversed=false") << false;
    QTest::newRow("inversed=true") << true;
}

void TestVPAPI::motion_copyAndPasteAndCutLightKeyframe()
{
    QFETCH(bool, inversed);
    ProjectProxy project;
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QObject *object = project.light();
    MotionProxy *parentMotion = project.light()->motion();
    project.setCurrentMotion(parentMotion);
    BaseMotionTrack *track = project.light()->track();
    testCopyAndPasteAndTest(project, track, object, inversed, 2, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_copyAndPasteAndCutBoneKeyframe_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::addColumn<bool>("inversed");
    QTest::newRow("PMD") << IModel::kPMDModel << false;
    QTest::newRow("PMX") << IModel::kPMXModel << false;
    QTest::newRow("PMD+inversed") << IModel::kPMDModel << true;
    QTest::newRow("PMX+inversed") << IModel::kPMXModel << true;
}

void TestVPAPI::motion_copyAndPasteAndCutBoneKeyframe()
{
    QFETCH(IModel::Type, modelType);
    QFETCH(bool, inversed);
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    BoneRefObject *bone = modelProxy->createBone();
    bone->setName(kBoneName);
    project.addModel(modelProxy);
    project.initializeMotion(modelProxy, ProjectProxy::ModelMotion);
    BoneMotionTrack *track = modelProxy->childMotion()->findBoneMotionTrack(bone);
    testCopyAndPasteAndTest(project, track, bone, inversed, 1, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_copyAndPasteAndCutMorphKeyframe_data()
{
    QTest::addColumn<IModel::Type>("modelType");
    QTest::addColumn<bool>("inversed");
    QTest::newRow("PMD") << IModel::kPMDModel << false;
    QTest::newRow("PMX") << IModel::kPMXModel << false;
    QTest::newRow("PMD+inversed") << IModel::kPMDModel << true;
    QTest::newRow("PMX+inversed") << IModel::kPMXModel << true;
}

void TestVPAPI::motion_copyAndPasteAndCutMorphKeyframe()
{
    QFETCH(IModel::Type, modelType);
    QFETCH(bool, inversed);
    ProjectProxy project;
    project.initializeOnce();
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    QScopedPointer<IModel> model(project.factoryInstanceRef()->newModel(modelType));
    ModelProxy *modelProxy = project.createModelProxy(model.take(), QUuid::createUuid(), QUrl());
    MorphRefObject *morph = modelProxy->createMorph();
    morph->setName(kMorphName);
    project.addModel(modelProxy);
    project.initializeMotion(modelProxy, ProjectProxy::ModelMotion);
    MorphMotionTrack *track = modelProxy->childMotion()->findMorphMotionTrack(morph);
    testCopyAndPasteAndTest(project, track, morph, inversed, 1, 0, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::motion_mergeCameraKeyframe()
{
    ProjectProxy project;
    QSignalSpy undoDidPerform(&project, SIGNAL(undoDidPerform()));
    QSignalSpy redoDidPerform(&project, SIGNAL(redoDidPerform()));
    QSignalSpy currentTimeIndexChanged(&project, SIGNAL(currentTimeIndexChanged()));
    project.initializeOnce();
    project.setCurrentMotion(project.camera()->motion());
}

void TestVPAPI::testAddKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *object, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged)
{
    track->parentMotion()->addKeyframe(object, kTimeIndex);
    testNewKeyframe(project, track, object, baseSize, baseChanged, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::testRemoveKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *object, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged)
{
    MotionProxy *parentMotion = track->parentMotion();
    BaseKeyframeRefObject *keyframe = parentMotion->resolveKeyframeAt(kTimeIndex, object);
    parentMotion->removeKeyframe(keyframe);
    testOldKeyframe(project, track, object, baseSize, baseChanged, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::testCopyAndPasteAndTest(ProjectProxy &project, BaseMotionTrack *track, QObject *object, bool inversed, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged)
{
    MotionProxy *parentMotion = track->parentMotion();
    BaseKeyframeRefObject *keyframe = parentMotion->resolveKeyframeAt(0, object);
    QList<BaseKeyframeRefObject *> keyframes; keyframes << keyframe;
    parentMotion->selectKeyframes(keyframes);
    parentMotion->copyKeyframes();
    parentMotion->pasteKeyframes(kTimeIndex, inversed);
    testNewKeyframe(project, track, object, baseSize, baseChanged, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
    BaseKeyframeRefObject *keyframe2 = parentMotion->resolveKeyframeAt(kTimeIndex, object);
    keyframes.clear(); keyframes << keyframe2;
    parentMotion->selectKeyframes(keyframes);
    parentMotion->copyKeyframes();
    parentMotion->cutKeyframes();
    testOldKeyframe(project, track, object, baseSize + 1, baseChanged + 2, undoDidPerform, redoDidPerform, currentTimeIndexChanged);
}

void TestVPAPI::testNewKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *object, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged)
{
    MotionProxy *parentMotion = track->parentMotion();
    BaseKeyframeRefObject *newKeyframeRef = parentMotion->resolveKeyframeAt(kTimeIndex, object);
    quint64 timeIndex = newKeyframeRef->timeIndex();
    QVERIFY(newKeyframeRef);
    QCOMPARE(timeIndex, quint64(kTimeIndex));
    QCOMPARE(track->keyframes().size(), baseSize + 1);
    QCOMPARE(parentMotion->data()->countKeyframes(track->type()), baseSize + 1);
    project.undo();
    QVERIFY(!project.canUndo());
    QCOMPARE(undoDidPerform.size(), 1);
    QCOMPARE(currentTimeIndexChanged.size(), baseChanged + 1);
    QCOMPARE(track->keyframes().size(), baseSize);
    QCOMPARE(track->parentMotion()->data()->countKeyframes(track->type()), baseSize);
    QVERIFY(!parentMotion->resolveKeyframeAt(kTimeIndex, object));
    project.redo();
    QVERIFY(!project.canRedo());
    QCOMPARE(redoDidPerform.size(), 1);
    QCOMPARE(currentTimeIndexChanged.size(), baseChanged + 2);
    QCOMPARE(track->keyframes().size(), baseSize + 1);
    QCOMPARE(parentMotion->data()->countKeyframes(track->type()), baseSize + 1);
    BaseKeyframeRefObject *redoKeyframe = parentMotion->resolveKeyframeAt(kTimeIndex, object);
    QVERIFY(redoKeyframe);
    QCOMPARE(redoKeyframe->timeIndex(), timeIndex);
}

void TestVPAPI::testOldKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *object, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged)
{
    MotionProxy *parentMotion = track->parentMotion();
    QCOMPARE(track->keyframes().size(), baseSize - 1);
    QCOMPARE(parentMotion->data()->countKeyframes(track->type()), baseSize - 1);
    QVERIFY(!parentMotion->resolveKeyframeAt(kTimeIndex, object));
    project.undo();
    BaseKeyframeRefObject *keyframe = parentMotion->resolveKeyframeAt(kTimeIndex, object);
    QCOMPARE(undoDidPerform.size(), 2);
    QCOMPARE(currentTimeIndexChanged.size(), baseChanged + 1);
    QCOMPARE(track->keyframes().size(), baseSize);
    QCOMPARE(parentMotion->data()->countKeyframes(track->type()), baseSize);
    QCOMPARE(parentMotion->resolveKeyframeAt(kTimeIndex, object), keyframe);
    project.redo();
    QCOMPARE(redoDidPerform.size(), 2);
    QCOMPARE(currentTimeIndexChanged.size(), baseChanged + 2);
    QCOMPARE(track->keyframes().size(), baseSize - 1);
    QCOMPARE(parentMotion->data()->countKeyframes(track->type()), baseSize - 1);
    QVERIFY(!parentMotion->resolveKeyframeAt(kTimeIndex, object));
}

QTEST_MAIN(TestVPAPI)
