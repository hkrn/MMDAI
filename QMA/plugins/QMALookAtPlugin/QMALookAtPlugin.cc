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

#include "QMALookAtPlugin.h"

#include <QtCore>
#include <MMDAI/MMDAI.h>

static void setNeckController(MMDAI::BoneController *controller, MMDAI::PMDModel *model)
{
    const char head[] = { 0x93, 0xaa, 0x0 }; /* 頭 in Shift_JIS */
    const char *bone[] = { head };
    controller->setup(model, bone, 1, 0.150f, 0.008f,
                      btVector3(0.0f, 0.0f, 1.0f),
                      btVector3(20.0f, 60.0f, 0.0f),
                      btVector3(-45.0f, -60.0f, 0.0f),
                      btVector3(0.0f, -1.0f, 0.0f));
}

static void setEyeController(MMDAI::BoneController *controller, MMDAI::PMDModel *model)
{
    const char rightEye[] = { 0x89, 0x45, 0x96, 0xda, 0x0 }; /* 右目 in Shift_JIS */
    const char leftEye[] = { 0x8d, 0xb6, 0x96, 0xda, 0x0 };  /* 左目 in Shift_JIS */
    const char *bone[] = { rightEye, leftEye };
    controller->setup(model, bone, 2, 0.180f, 0.008f,
                      btVector3(0.0f, 0.0f, 1.0f),
                      btVector3(5.0f, 5.0f, 0.0f),
                      btVector3(-5.0f, -5.0f, 0.0f),
                      btVector3(0.0f, 0.0f, 0.0f));
}

QMALookAtPlugin::QMALookAtPlugin()
    : m_enable(false),
      m_maxModel(0),
      m_controller(0),
      m_neckController(0),
      m_eyeController(0)
{
}

QMALookAtPlugin::~QMALookAtPlugin()
{
}

void QMALookAtPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(baseName);
    int maxModel = m_maxModel = controller->getMaxObjects();
    m_controller = controller;
    m_eyeController = new MMDAI::BoneController[maxModel];
    m_neckController = new MMDAI::BoneController[maxModel];
}

void QMALookAtPlugin::unload()
{
    delete[] m_eyeController;
    m_eyeController = 0;
    delete[] m_neckController;
    m_neckController = 0;
}

void QMALookAtPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    Q_UNUSED(command);
    Q_UNUSED(arguments);
    /* do nothing */
}

void QMALookAtPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    if (type == QMAPlugin::getUpdateEvent()) {
        QRect rect = arguments[0].toRect();
        QPoint pos = arguments[1].toPoint(), p = pos;
        double delta = arguments[2].toDouble();
        btVector3 pointPos;
        /* set target position */
        p.setX(pos.x() - ((rect.left() + rect.right()) / 2));
        p.setY(pos.y() - ((rect.top() + rect.bottom()) / 2));
        float rate = 100.0f / static_cast<float>(rect.right() - rect.left());
        pointPos.setValue(p.x() * rate, -p.y() * rate, 0.0f);
        btVector3 targetPos = m_controller->getScreenPointPosition(pointPos);
        /* calculate direction of all controlled bones */
        for (int i = 0; i < m_maxModel; i++) {
            MMDAI::PMDObject *object = m_controller->getObjectAt(i);
            if (object && object->isEnable()) {
                m_neckController[i].update(targetPos, static_cast<float>(delta));
                m_eyeController[i].update(targetPos, static_cast<float>(delta));
            }
        }
    }
    else if (type == MMDAI::ISceneEventHandler::kKeyEvent && arguments.at(0).toString() == "L") {
        m_enable = !m_enable;
        for (int i = 0; i < m_maxModel; i++) {
            MMDAI::PMDObject *object = m_controller->getObjectAt(i);
            if (object && object->isEnable()) {
                m_neckController[i].setEnable(m_enable);
                m_eyeController[i].setEnable(m_enable);
            }
        }
    }
    else if (type == MMDAI::ISceneEventHandler::kModelChangeEvent || type == MMDAI::ISceneEventHandler::kModelAddEvent) {
        QTextCodec *codec = QTextCodec::codecForName("UTF8");
        QString target = arguments.at(0).toString();
        for (int i = 0; i < m_maxModel; i++) {
            const char *a;
            MMDAI::PMDObject *object = m_controller->getObjectAt(i);
            if (object && ((a = object->getAlias()) != NULL)) {
                QString alias = codec->toUnicode(a, MMDAIStringLength(a));
                if (alias == target) {
                    MMDAI::PMDModel *model = object->getModel();
                    setNeckController(&m_neckController[i], model);
                    setEyeController(&m_eyeController[i], model);
                }
            }
        }
    }
}

Q_EXPORT_PLUGIN2(qma_lookat_plugin, QMALookAtPlugin);
