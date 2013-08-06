/**

 Copyright (c) 2010-2013  hkrn

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

#pragma once
#ifndef VPVL2_EXTENSIONS_UI_ANTTWEAKBAR_H_
#define VPVL2_EXTENSIONS_UI_ANTTWEAKBAR_H_

#include <vpvl2/Common.h>
#include <vpvl2/IBone.h>
#include <vpvl2/IModel.h>
#include <vpvl2/IMorph.h>
#include <vpvl2/extensions/BaseApplicationContext.h>

#include <sstream>
#include <AntTweakBar.h>

namespace vpvl2
{
namespace extensions
{
namespace ui
{

class AntTweakBar {
public:
    static void initialize(bool enableCoreProfile) {
        ::atexit(&AntTweakBar::terminate);
        TwInit(enableCoreProfile ? TW_OPENGL_CORE : TW_OPENGL, 0);
    }

    AntTweakBar() {
    }
    ~AntTweakBar() {
    }

    void create(IApplicationContext *applicationContext) {
        TwBar *sceneBar = TwNewBar("Scene");
        TwDefine("GLOBAL help=''");
        TwDefine("Scene label='Scene control UI' size='220 400' valuesWidth=fit movable=true resizable=true");
        TwAddVarCB(sceneBar, "TimeIndex", TW_TYPE_DOUBLE, 0, getTimeIndex, applicationContext,
                   "group='Time' label='TimeIndex' precision=0 help='Show current time index of scene.'");
        TwDefine("Scene/Time label='Time'");
        TwAddSeparator(sceneBar, "Separator0", "");
        TwAddVarCB(sceneBar, "Angle", TW_TYPE_DIR3F, setCameraAngle, getCameraAngle, applicationContext,
                   "group='Camera' label='Angle' help='Change angle of current camera.'");
        TwAddVarCB(sceneBar, "FOV", TW_TYPE_FLOAT, setCameraFOV, getCameraFOV, applicationContext,
                   "group='Camera' label='FOV' min=10 max=135 step=0.1 help='Change FOV (Field Of View) of current camera.'");
        TwAddVarCB(sceneBar, "Distance", TW_TYPE_FLOAT, setCameraDistance, getCameraDistance, applicationContext,
                   "group='Camera' label='Distance' step=1 keyIncr='+' keyDecr='-' help='Change distance from current camera look at position.'");
        TwDefine("Scene/Camera label='Camera'");
        TwAddSeparator(sceneBar, "Separator1", "");
        TwAddVarCB(sceneBar, "Color", TW_TYPE_COLOR3F, setLightColor, getLightColor, applicationContext,
                   "group='Light' label='Color' help='Change color of current light.'");
        TwAddVarCB(sceneBar, "Direction", TW_TYPE_DIR3F, setLightDirection, getLightDirection, applicationContext,
                   "group='Light' label='Direction' help='Change direction of current light.'");
        TwDefine("Scene/Light label='Light'");
    }
    bool handleAction(IApplicationContext::MousePositionType type, bool pressed) {
        TwMouseAction actionType = pressed ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
        bool handled = false;
        switch (type) {
        case IApplicationContext::kMouseLeftPressPosition:
            handled = TwMouseButton(actionType, TW_MOUSE_LEFT) != 0;
            break;
        case IApplicationContext::kMouseMiddlePressPosition:
            handled = TwMouseButton(actionType, TW_MOUSE_MIDDLE) != 0;
            break;
        case IApplicationContext::kMouseRightPressPosition:
            handled = TwMouseButton(actionType, TW_MOUSE_RIGHT) != 0;
            break;
        case IApplicationContext::kMouseCursorPosition:
        default:
            break;
        }
        return handled;
    }
    void setCurrentModelRef(IModel *value) {
        TwDeleteBar(TwGetBarByName("CurrentModel"));
        if (value) {
            TwBar *currentModelBar = TwNewBar("CurrentModel");
            TwAddVarCB(currentModelBar, "ModelVersion", TW_TYPE_FLOAT, 0, getModelVersion, value, "group='Property' label='Version' precision=1");
            TwAddVarCB(currentModelBar, "ModelName", TW_TYPE_STDSTRING, 0, getModelName, value, "group='Property' label='Name' help='Show the name of current model.'");
            TwAddVarCB(currentModelBar, "ModelWorldPositionX", TW_TYPE_FLOAT, setModelXPosition, getModelXPosition, value, "group='Property' step=0.01 label='X Position' help='Change world X position of current model.'");
            TwAddVarCB(currentModelBar, "ModelWorldPositionY", TW_TYPE_FLOAT, setModelYPosition, getModelYPosition, value, "group='Property' step=0.01 label='Y Position' help='Change world Y position of current model.'");
            TwAddVarCB(currentModelBar, "ModelWorldPositionZ", TW_TYPE_FLOAT, setModelZPosition, getModelZPosition, value, "group='Property' step=0.01 label='Z Position' help='Change world Z position of current model.'");
            TwAddVarCB(currentModelBar, "ModelWorldRotation", TW_TYPE_QUAT4F, setModelRotation, getModelRotation, value, "group='Property' label='Rotation' help='Change world rotation of current model.'");
            TwAddVarCB(currentModelBar, "ModelScaleFactor", TW_TYPE_DOUBLE, setModelScaleFactor, getModelScaleFactor, value, "group='Property' step=0.01 label='Scale Factor' help='Change scale factor of current model.'");
            TwAddVarCB(currentModelBar, "ModelEdgeWidth", TW_TYPE_DOUBLE, setModelEdgeWidth, getModelEdgeWidth, value, "group='Property' min=0 max=2 step=0.01 label='Edge width' help='Change edge width of current model.'");
            TwAddVarCB(currentModelBar, "ModelOpacity", TW_TYPE_DOUBLE, setModelOpacity, getModelOpacity, value, "group='Property' min=0 max=1 step=0.01 label='Opacity' help='Change opacity of current model.'");
            TwAddVarCB(currentModelBar, "ModelVisibility", TW_TYPE_BOOL8, setModelVisible, getModelIsVisible, value, "group='Property' label='Visible' help='Toggle visibility of current model.'");
            TwAddVarCB(currentModelBar, "ModelBoneCountProperty", TW_TYPE_INT32, 0, getModelBoneCountProperty, value, "group='ObjectCount' label='Bones'");
            TwAddVarCB(currentModelBar, "ModelIKCountProperty", TW_TYPE_INT32, 0, getModelIKCountProperty, value, "group='ObjectCount' label='IKs'");
            TwAddVarCB(currentModelBar, "ModelIndexCountProperty", TW_TYPE_INT32, 0, getModelIndexCountProperty, value, "group='ObjectCount' label='Indices'");
            TwAddVarCB(currentModelBar, "ModelJointCountProperty", TW_TYPE_INT32, 0, getModelJointCountProperty, value, "group='ObjectCount' label='Joints'");
            TwAddVarCB(currentModelBar, "ModelMaterialCountProperty", TW_TYPE_INT32, 0, getModelMaterialCountProperty, value, "group='ObjectCount' label='Materials'");
            TwAddVarCB(currentModelBar, "ModelMorphCountProperty", TW_TYPE_INT32, 0, getModelMorphCountProperty, value, "group='ObjectCount' label='Morphs'");
            TwAddVarCB(currentModelBar, "ModelRigidBodyCountProperty", TW_TYPE_INT32, 0, getModelRigidBodyCountProperty, value, "group='ObjectCount' label='RigidBodies'");
            TwAddVarCB(currentModelBar, "ModelTextureCountProperty", TW_TYPE_INT32, 0, getModelTextureCountProperty, value, "group='ObjectCount' label='Textures'");
            TwAddVarCB(currentModelBar, "ModelVertexCountProperty", TW_TYPE_INT32, 0, getModelVertexCountProperty, value, "group='ObjectCount' label='Vertices'");
            TwDefine("CurrentModel/ObjectCount group='Property' opened=true label='Number of objects'");
            TwDefine("CurrentModel/Property opened=true label='Property'");
            buildBoneList(currentModelBar, value);
            buildMorphList(currentModelBar, value);
            TwDefine("CurrentModel label='Model' size='300 500' valuesWidth=fit movable=true resizable=true");
        }
    }
    bool handleWheel(int delta) {
        return TwMouseWheel(delta) != 0;
    }
    bool handleMotion(int x, int y) {
        return TwMouseMotion(x, y) != 0;
    }
    bool handleKeycode(int key, int modifiers) {
        return TwKeyTest(key, modifiers) != 0;
    }

    void resize(int width, int height) {
        TwWindowSize(width, height);
    }
    void render() {
        TwDraw();
    }

private:
    static inline void terminate() {
        TwTerminate();
    }
    static inline void assignX(void *value, const Vector3 &v) {
        *static_cast<float32 *>(value) = v.x();
    }
    static inline void assignY(void *value, const Vector3 &v) {
        *static_cast<float32 *>(value) = v.y();
    }
    static inline void assignZ(void *value, const Vector3 &v) {
        *static_cast<float32 *>(value) = v.z();
    }
    static inline Vector3 setX(const void *value, const Vector3 &v) {
        Vector3 v2(v);
        v2.setX(*static_cast<const float32 *>(value));
        return v2;
    }
    static inline Vector3 setY(const void *value, const Vector3 &v) {
        Vector3 v2(v);
        v2.setY(*static_cast<const float32 *>(value));
        return v2;
    }
    static inline Vector3 setZ(const void *value, const Vector3 &v) {
        Vector3 v2(v);
        v2.setZ(*static_cast<const float32 *>(value));
        return v2;
    }
    static inline void TW_CALL getModelObjectCountProperty(void *value, void *userData, IModel::ObjectType type) {
        IModel *modelRef = static_cast<IModel *>(userData);
        *static_cast<int *>(value) = modelRef->count(type);
    }
    static inline void TW_CALL getTimeIndex(void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            *static_cast<IKeyframe::TimeIndex *>(value) = sceneRef->currentTimeIndex();
        }
    }
    static inline void TW_CALL getCameraAngle(void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            const Vector3 &angle = sceneRef->cameraRef()->angle();
            float32 *v = static_cast<float32 *>(value);
            v[0] = angle.x();
            v[1] = angle.y();
            v[2] = angle.z();
        }
    }
    static inline void TW_CALL setCameraAngle(const void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            const float32 *v = static_cast<const float32 *>(value);
            sceneRef->cameraRef()->setAngle(Vector3(v[0], v[1], v[2]));
        }
    }
    static inline void TW_CALL getCameraFOV(void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            *static_cast<float32 *>(value) = sceneRef->cameraRef()->fov();
        }
    }
    static inline void TW_CALL setCameraFOV(const void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            sceneRef->cameraRef()->setFov(*static_cast<const float32 *>(value));
        }
    }
    static inline void TW_CALL getCameraDistance(void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            *static_cast<float32 *>(value) = sceneRef->cameraRef()->distance();
        }
    }
    static inline void TW_CALL setCameraDistance(const void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            sceneRef->cameraRef()->setDistance(*static_cast<const float32 *>(value));
        }
    }
    static inline void TW_CALL getLightColor(void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            const Vector3 &color = sceneRef->lightRef()->color();
            float32 *v = static_cast<float32 *>(value);
            v[0] = color.x();
            v[1] = color.y();
            v[2] = color.z();
        }
    }
    static inline void TW_CALL setLightColor(const void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            const float32 *v = static_cast<const float32 *>(value);
            sceneRef->lightRef()->setColor(Vector3(v[0], v[1], v[2]));
        }
    }
    static inline void TW_CALL getLightDirection(void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            const Vector3 &direction = sceneRef->lightRef()->direction();
            float32 *v = static_cast<float32 *>(value);
            v[0] = direction.x();
            v[1] = direction.y();
            v[2] = direction.z();
        }
    }
    static inline void TW_CALL setLightDirection(const void *value, void *userData) {
        const BaseApplicationContext *context = static_cast<const BaseApplicationContext *>(userData);
        if (const Scene *sceneRef = context->sceneRef()) {
            const float32 *v = static_cast<const float32 *>(value);
            sceneRef->lightRef()->setDirection(Vector3(v[0], v[1], v[2]));
        }
    }
    static inline void TW_CALL getBoneRotation(void *value, void *userData) {
        const IBone *boneRef = static_cast<const IBone *>(userData);
        const Quaternion &rotation = boneRef->localOrientation();
        float32 *v = static_cast<float32 *>(value);
        v[0] = rotation.x();
        v[1] = rotation.y();
        v[2] = rotation.z();
        v[3] = rotation.w();
    }
    static inline void TW_CALL setBoneRotation(const void *value, void *userData) {
        IBone *boneRef = static_cast<IBone *>(userData);
        const float32 *v = static_cast<const float32 *>(value);
        boneRef->setLocalOrientation(Quaternion(v[0], v[1], v[2], v[3]));
    }
    static inline void TW_CALL getBoneXPosition(void *value, void *userData) {
        const IBone *boneRef = static_cast<const IBone *>(userData);
        assignX(value, boneRef->localTranslation());
    }
    static inline void TW_CALL setBoneXPosition(const void *value, void *userData) {
        IBone *boneRef = static_cast<IBone *>(userData);
        boneRef->setLocalTranslation(setX(value, boneRef->localTranslation()));
    }
    static inline void TW_CALL getBoneYPosition(void *value, void *userData) {
        const IBone *boneRef = static_cast<const IBone *>(userData);
        assignY(value, boneRef->localTranslation());
    }
    static inline void TW_CALL setBoneYPosition(const void *value, void *userData) {
        IBone *boneRef = static_cast<IBone *>(userData);
        boneRef->setLocalTranslation(setY(value, boneRef->localTranslation()));
    }
    static inline void TW_CALL getBoneZPosition(void *value, void *userData) {
        const IBone *boneRef = static_cast<const IBone *>(userData);
        assignZ(value, boneRef->localTranslation());
    }
    static inline void TW_CALL setBoneZPosition(const void *value, void *userData) {
        IBone *boneRef = static_cast<IBone *>(userData);
        boneRef->setLocalTranslation(setZ(value, boneRef->localTranslation()));
    }
    static inline void TW_CALL getMorphWeight(void *value, void *userData) {
        const IMorph *morphRef = static_cast<const IMorph *>(userData);
        *static_cast<double *>(value) = morphRef->weight();
    }
    static inline void TW_CALL setMorphWeight(const void *value, void *userData) {
        IMorph *morphRef = static_cast<IMorph *>(userData);
        morphRef->setWeight(*static_cast<const double *>(value));
    }
    static inline void TW_CALL getModelEdgeWidth(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        *static_cast<double *>(value) = modelRef->edgeWidth();
    }
    static inline void TW_CALL setModelEdgeWidth(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        modelRef->setEdgeWidth(*static_cast<const double *>(value));
    }
    static inline void TW_CALL getModelScaleFactor(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        *static_cast<double *>(value) = modelRef->scaleFactor();
    }
    static inline void TW_CALL setModelScaleFactor(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        modelRef->setScaleFactor(*static_cast<const double *>(value));
    }
    static inline void TW_CALL getModelXPosition(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        assignX(value, modelRef->worldTranslation());
    }
    static inline void TW_CALL setModelXPosition(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        modelRef->setWorldTranslation(setX(value, modelRef->worldTranslation()));
    }
    static inline void TW_CALL getModelYPosition(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        assignY(value, modelRef->worldTranslation());
    }
    static inline void TW_CALL setModelYPosition(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        modelRef->setWorldTranslation(setY(value, modelRef->worldTranslation()));
    }
    static inline void TW_CALL getModelZPosition(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        assignZ(value, modelRef->worldTranslation());
    }
    static inline void TW_CALL setModelZPosition(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        modelRef->setWorldTranslation(setZ(value, modelRef->worldTranslation()));
    }
    static inline void TW_CALL getModelRotation(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        const Quaternion &rotation = modelRef->worldOrientation();
        float32 *v = static_cast<float32 *>(value);
        v[0] = rotation.x();
        v[1] = rotation.y();
        v[2] = rotation.z();
        v[3] = rotation.w();
    }
    static inline void TW_CALL setModelRotation(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        const float32 *v = static_cast<const float32 *>(value);
        modelRef->setWorldOrientation(Quaternion(v[0], v[1], v[2], v[3]));
    }
    static inline void TW_CALL getModelOpacity(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        *static_cast<double *>(value) = modelRef->opacity();
    }
    static inline void TW_CALL setModelOpacity(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        modelRef->setOpacity(*static_cast<const double *>(value));
    }
    static inline void TW_CALL getModelIsVisible(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        *static_cast<uint8_t *>(value) = modelRef->isVisible() != 0;
    }
    static inline void TW_CALL setModelVisible(const void *value, void *userData) {
        IModel *modelRef = static_cast<IModel *>(userData);
        modelRef->setVisible(*static_cast<const uint8_t *>(value) != 0);
    }
    static inline void TW_CALL getModelBoneCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kBone);
    }
    static inline void TW_CALL getModelIKCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kIK);
    }
    static inline void TW_CALL getModelIndexCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kIndex);
    }
    static inline void TW_CALL getModelJointCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kJoint);
    }
    static inline void TW_CALL getModelMaterialCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kMaterial);
    }
    static inline void TW_CALL getModelMorphCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kMorph);
    }
    static inline void TW_CALL getModelRigidBodyCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kRigidBody);
    }
    static inline void TW_CALL getModelVertexCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kVertex);
    }
    static inline void TW_CALL getModelTextureCountProperty(void *value, void *userData) {
        getModelObjectCountProperty(value, userData, IModel::kTexture);
    }
    static inline void TW_CALL getModelVersion(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        *static_cast<float *>(value) = modelRef->version();
    }
    static inline void TW_CALL getModelName(void *value, void *userData) {
        const IModel *modelRef = static_cast<const IModel *>(userData);
        const icu4c::String *name = static_cast<const icu4c::String *>(modelRef->name(IEncoding::kEnglish));
        std::string *ptr = static_cast<std::string *>(value);
        TwCopyStdStringToLibrary(*ptr, icu4c::String::toStdString(name->value()));
    }
    static void buildBoneList(TwBar *currentModelBar, const IModel *modelRef) {
        Array<ILabel *> labels;
        modelRef->getLabelRefs(labels);
        const int nlabels = labels.count();
        std::ostringstream nameStringStream, groupStringStream;
        std::string nameString, definitionString, groupString, labelString, variableNameString;
        for (int i = 0; i < nlabels; i++) {
            const ILabel *labelRef = labels[i];
            int labelIndex = labelRef->index();
            groupStringStream.str(std::string());
            groupStringStream << "Label" << labelIndex;
            groupString.assign(groupStringStream.str());
            const int nbones = labelRef->count();
            int found = 0;
            for (int j = 0; j < nbones; j++) {
                IBone *boneRef = labelRef->boneRef(j);
                if (boneRef && boneRef->isVisible()) {
                    int boneIndex = boneRef->index();
                    nameStringStream.str(std::string());
                    nameStringStream << "Bone" << boneIndex;
                    nameString.assign(nameStringStream.str());
                    const IString *englishName = boneRef->name(IEncoding::kEnglish);
                    if (englishName && englishName->size() > 0) {
                        labelString.assign(icu4c::String::toStdString(static_cast<const icu4c::String *>(englishName)->value()));
                    }
                    else {
                        labelString.assign("Unknown (").append(nameString).append(")");
                    }
                    variableNameString.assign(groupString).append(nameString).append("Rotation");
                    definitionString.assign("group='").append(nameString).append("' label='Rotation' opened=true");
                    TwAddVarCB(currentModelBar, variableNameString.c_str(), TW_TYPE_QUAT4F, setBoneRotation, getBoneRotation, boneRef, definitionString.c_str());
                    if (boneRef->isMovable()) {
                        variableNameString.assign(groupString).append(nameString).append("PositionX");
                        definitionString.assign("group='").append(nameString).append("' label='X Position' step=0.01");
                        TwAddVarCB(currentModelBar, variableNameString.c_str(), TW_TYPE_FLOAT, setBoneXPosition, getBoneXPosition, boneRef, definitionString.c_str());
                        variableNameString.assign(groupString).append(nameString).append("PositionY");
                        definitionString.assign("group='").append(nameString).append("' label='Y Position' step=0.01");
                        TwAddVarCB(currentModelBar, variableNameString.c_str(), TW_TYPE_FLOAT, setBoneYPosition, getBoneYPosition, boneRef, definitionString.c_str());
                        variableNameString.assign(groupString).append(nameString).append("PositionZ");
                        definitionString.assign("group='").append(nameString).append("' label='Z Position' step=0.01");
                        TwAddVarCB(currentModelBar, variableNameString.c_str(), TW_TYPE_FLOAT, setBoneZPosition, getBoneZPosition, boneRef, definitionString.c_str());
                    }
                    definitionString.assign("CurrentModel/").append(nameString).append(" label='").append(labelString).append("' group='").append(groupString).append("' opened=false");
                    TwDefine(definitionString.c_str());
                    found++;
                }
            }
            if (found > 0) {
                const IString *englishName = labelRef->name(IEncoding::kEnglish);
                if (englishName && englishName->size() > 0) {
                    labelString.assign(icu4c::String::toStdString(static_cast<const icu4c::String *>(englishName)->value()));
                }
                else {
                    labelString.assign("Unknown (").append(groupString).append(")");
                }
                definitionString.assign("CurrentModel/").append(groupString).append(" label='").append(labelString).append("' opened=false");
                TwDefine(definitionString.c_str());
            }
        }
    }
    static void buildMorphList(TwBar *currentModelBar, const IModel *modelRef) {
        std::ostringstream nameStringStream;
        std::string nameString, definitionString, labelString, variableNameString;
        Array<IMorph *> morphs;
        modelRef->getMorphRefs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            IMorph *morphRef = morphs[i];
            int morphIndex = morphRef->index();
            nameStringStream.str(std::string());
            nameStringStream << "Morph" << morphIndex;
            nameString.assign(nameStringStream.str());
            const IString *englishName = morphRef->name(IEncoding::kEnglish);
            if (englishName && englishName->size() > 0) {
                labelString.assign(icu4c::String::toStdString(static_cast<const icu4c::String *>(englishName)->value()));
            }
            else {
                labelString.assign("Unknown (").append(nameString).append(")");
            }
            variableNameString.assign("Morph").append(nameString).append("Weight");
            definitionString.assign("group='Morph' min=0 max=1 step=0.01 label='").append(labelString).append("'");
            TwAddVarCB(currentModelBar, variableNameString.c_str(), TW_TYPE_DOUBLE, setMorphWeight, getMorphWeight, morphRef, definitionString.c_str());
        }
        TwDefine("CurrentModel/Morph opened=false");
    }

};

} /* namespace ui */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
