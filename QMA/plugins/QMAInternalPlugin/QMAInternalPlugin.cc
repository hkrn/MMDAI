/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

/* under construction */

#include "QMAInternalPlugin.h"

#include <QDebug>
#include <QStringList>
#include <QTextCodec>
#include <QVector3D>
#include <QVector4D>

#include <MMDAI/MMDAI.h>

const QString QMAInternalPlugin::kAllObjectNames = "MMDAI::Internal::allObjectNames";
const QString QMAInternalPlugin::kAllObjectNamesEvent = "MMDAI::Internal::allObjectNamesEvent";
const QString QMAInternalPlugin::kAllBoneNames = "MMDAI::Internal::allBoneNames";
const QString QMAInternalPlugin::kAllBoneNamesEvent = "MMDAI::Internal::allBoneNamesEvent";
const QString QMAInternalPlugin::kAllFaceNames = "MMDAI::Internal::allFaceNames";
const QString QMAInternalPlugin::kAllFaceNamesEvent = "MMDAI::Internal::allFaceNamesEvent";
const QString QMAInternalPlugin::kBoneTransform = "MMDAI::Internal::boneTransform";
const QString QMAInternalPlugin::kFaceSetWeight = "MMDAI::Internal::faceSetWeight";

QMAInternalPlugin::QMAInternalPlugin(QObject *parent)
    : QMAPlugin(parent),
      m_codec(QTextCodec::codecForName("Shift-JIS")),
      m_controller(0)
{
}

QMAInternalPlugin::~QMAInternalPlugin()
{
    m_controller = 0;
}

void QMAInternalPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(baseName);
    m_controller = controller;
}

void QMAInternalPlugin::unload()
{
}

void QMAInternalPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    QList<QVariant> returns;
    int argc = arguments.count();
    if (command == kAllObjectNames) {
        QStringList modelNames;
        int nmodels = m_controller->countPMDObjects();
        for (int i = 0; i < nmodels; i++) {
            MMDAI::PMDObject *object = m_controller->getPMDObject(i);
            if (object != NULL && object->isEnable()) {
                modelNames << object->getPMDModel()->getName();
            }
        }
        returns << modelNames;
        eventPost(kAllObjectNamesEvent, returns);
    }
    else if (command == kAllBoneNames && argc >= 1) {
        MMDAI::PMDModel *model = findModel(arguments[0].toString());
        if (model != NULL) {
            QStringList boneNames;
            int nbones = model->countBones();
            for (int i = 0; i < nbones; i++) {
                MMDAI::PMDBone *bone = model->getBoneAt(i);
                boneNames << m_codec->toUnicode(bone->getName());
            }
            returns << boneNames;
            eventPost(kAllBoneNamesEvent, returns);
        }
    }
    else if (command == kAllFaceNames && argc >= 1) {
        MMDAI::PMDModel *model = findModel(arguments[0].toString());
        if (model != NULL) {
            QStringList faceNames;
            int nfaces = model->countFaces();
            for (int i = 0; i < nfaces; i++) {
                MMDAI::PMDFace *face = model->getFaceAt(i);
                faceNames << m_codec->toUnicode(face->getName());
            }
            returns << faceNames;
            eventPost(kAllFaceNamesEvent, returns);
        }
    }
    else if (command == kBoneTransform && argc >= 4) {
        MMDAI::PMDBone *bone = findBone(arguments[0].toString(), arguments[1].toString());
        if (bone != NULL) {
            btVector3 pos = bone->getCurrentPosition();
            btQuaternion rot = bone->getCurrentRotation();
            QVector3D vec3 = arguments[2].value<QVector3D>();
            QVector4D vec4 = arguments[3].value<QVector4D>();
            pos.setX(pos.x() + vec3.x());
            pos.setY(pos.y() + vec3.y());
            pos.setZ(pos.z() + vec3.z());
            rot.setX(pos.x() + vec4.x());
            rot.setY(pos.y() + vec4.y());
            rot.setZ(pos.z() + vec4.z());
            rot.setW(pos.w() + vec4.w());
            bone->setCurrentPosition(pos);
            bone->setCurrentRotation(rot);
        }
    }
    else if (command == kFaceSetWeight && argc >= 3) {
        MMDAI::PMDFace *face = findFace(arguments[0].toString(), arguments[1].toString());
        if (face != NULL) {
            float value = qMax(qMin(arguments[2].toFloat(), 1.0f), 0.0f);
            face->setWeight(value);
        }
    }
}

void QMAInternalPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    Q_UNUSED(type);
    Q_UNUSED(arguments);
    /* do nothing */
}

MMDAI::PMDModel *QMAInternalPlugin::findModel(const QString &modelName)
{
    qDebug() << modelName;
    int nmodels = m_controller->countPMDObjects();
    for (int i = 0; i < nmodels; i++) {
        MMDAI::PMDObject *object = m_controller->getPMDObject(i);
        if (object != NULL && object->isEnable()) {
            MMDAI::PMDModel *model = object->getPMDModel();
            const QString name = m_codec->toUnicode(model->getName());
            qDebug() << name;
            if (name == modelName) {
                return model;
            }
        }
    }
    return NULL;
}

MMDAI::PMDBone *QMAInternalPlugin::findBone(const QString &modelName, const QString &boneName)
{
    MMDAI::PMDModel *model = findModel(modelName);
    if (model != NULL) {
        int nbones = model->countBones();
        for (int i = 0; i < nbones; i++) {
            MMDAI::PMDBone *bone = model->getBoneAt(i);
            if (bone != NULL) {
                const QString name = m_codec->toUnicode(bone->getName());
                if (name == boneName) {
                    return bone;
                }
            }
        }
    }
    return NULL;
}

MMDAI::PMDFace *QMAInternalPlugin::findFace(const QString &modelName, const QString &faceName)
{
    MMDAI::PMDModel *model = findModel(modelName);
    if (model != NULL) {
        int nfaces = model->countFaces();
        for (int i = 0; i < nfaces; i++) {
            MMDAI::PMDFace *face = model->getFaceAt(i);
            if (face != NULL) {
                const QString name = m_codec->toUnicode(face->getName());
                if (name == faceName) {
                    return face;
                }
            }
        }
    }
    return NULL;
}

Q_EXPORT_PLUGIN2(qma_internal_plugin, QMAInternalPlugin)
