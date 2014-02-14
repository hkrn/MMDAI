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

#include <QtTest>
#include <vpvl2/IModel.h>
#include <vpvl2/IMotion.h>
#include "ProjectProxy.h"

class BaseMotionTrack;

class TestVPAPI : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void project_initialize_data();
    void project_initialize();
    void project_create_data();
    void project_create();
    void project_createModelProxy_data();
    void project_createModelProxy();
    void project_createMotionProxy_data();
    void project_createMotionProxy();
    void project_addModelProxy_data();
    void project_addModelProxy();
    void project_initializeMotion_data();
    void project_initializeMotion();
    void project_seek_data();
    void project_seek();
    void project_rewind_data();
    void project_rewind();
    void project_reset_data();
    void project_reset();
    void project_deleteModel_data();
    void project_deleteModel();
    void project_deleteMotion_data();
    void project_deleteMotion();
    void model_addAndRemoveVertex_data();
    void model_addAndRemoveVertex();
    void model_addAndRemoveMaterial_data();
    void model_addAndRemoveMaterial();
    void model_addAndRemoveBone_data();
    void model_addAndRemoveBone();
    void model_addAndRemoveMorph_data();
    void model_addAndRemoveMorph();
    void model_addAndRemoveLabel_data();
    void model_addAndRemoveLabel();
    void model_addAndRemoveRigidBody_data();
    void model_addAndRemoveRigidBody();
    void model_addAndRemoveJoint_data();
    void model_addAndRemoveJoint();
    void model_translateTransform_data();
    void model_translateTransform();
    void model_rotateTransform_data();
    void model_rotateTransform();
    void model_release_data();
    void model_release();
    void motion_addAndRemoveCameraKeyframe_data();
    void motion_addAndRemoveCameraKeyframe();
    void motion_addAndUpdateCameraKeyframe_data();
    void motion_addAndUpdateCameraKeyframe();
    void motion_addAndRemoveLightKeyframe_data();
    void motion_addAndRemoveLightKeyframe();
    void motion_addAndUpdateLightKeyframe_data();
    void motion_addAndUpdateLightKeyframe();
    void motion_addAndRemoveBoneKeyframe_data();
    void motion_addAndRemoveBoneKeyframe();
    void motion_addAndUpdateBoneKeyframe_data();
    void motion_addAndUpdateBoneKeyframe();
    void motion_addAndRemoveMorphKeyframe_data();
    void motion_addAndRemoveMorphKeyframe();
    void motion_addAndUpdateMorphKeyframe_data();
    void motion_addAndUpdateMorphKeyframe();
    void motion_copyAndPasteAndCutCameraKeyframe_data();
    void motion_copyAndPasteAndCutCameraKeyframe();
    void motion_copyAndPasteAndCutLightKeyframe_data();
    void motion_copyAndPasteAndCutLightKeyframe();
    void motion_copyAndPasteAndCutBoneKeyframe_data();
    void motion_copyAndPasteAndCutBoneKeyframe();
    void motion_copyAndPasteAndCutMorphKeyframe_data();
    void motion_copyAndPasteAndCutMorphKeyframe();
    void motion_mergeCameraKeyframe();
    void motion_mergeLightKeyframe();
    void motion_mergeBoneKeyframe_data();
    void motion_mergeBoneKeyframe();
    void motion_mergeMorphKeyframe_data();
    void motion_mergeMorphKeyframe();

private:
    void testAddKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *opaque, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged);
    void testRemoveKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *opaque, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged);
    void testCopyAndPasteAndTest(ProjectProxy &project, BaseMotionTrack *track, QObject *opaque, bool inversed, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged);
    void testNewKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *opaque, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged);
    void testOldKeyframe(ProjectProxy &project, BaseMotionTrack *track, QObject *opaque, int baseSize, int baseChanged, const QSignalSpy &undoDidPerform, const QSignalSpy &redoDidPerform, const QSignalSpy &currentTimeIndexChanged);
};

Q_DECLARE_METATYPE(vpvl2::IModel::Type)
Q_DECLARE_METATYPE(vpvl2::IMotion::FormatType)
Q_DECLARE_METATYPE(ProjectProxy::LanguageType)
