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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#include <QTextCodec>

#include <MMDAI/MMDAI.h>
#include "QMALookAtPlugin.h"

static void setNeckController(MMDAI::BoneController *controller, MMDAI::PMDModel *model)
{
    const char head[] = { 0x93, 0xaa, 0x0 }; /* 頭 in Shift_JIS */
    const char *bone[] = { head };
    controller->setup(model, bone, 1, 0.150f, 0.008f, 0.0f, 0.0f, 1.0f, 20.0f, 60.0f, 0.0f, -45.0f, -60.0f, 0.0f, 0.0f, -1.0f, 0.0f);
}

static void setEyeController(MMDAI::BoneController *controller, MMDAI::PMDModel *model)
{
    const char rightEye[] = { 0x89, 0x45, 0x96, 0xda, 0x0 }; /* 右目 in Shift_JIS */
    const char leftEye[] = { 0x8d, 0xb6, 0x96, 0xda, 0x0 };  /* 左目 in Shift_JIS */
    const char *bone[] = { rightEye, leftEye };
    controller->setup(model, bone, 2, 0.180f, 0.008f, 0.0f, 0.0f, 1.0f, 5.0f, 5.0f, 0.0f, -5.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

QMALookAtPlugin::QMALookAtPlugin()
    : m_enable(false)
{
}

QMALookAtPlugin::~QMALookAtPlugin()
{
}

void QMALookAtPlugin::initialize(MMDAI::SceneController *controller)
{
    m_controller = controller;
}

void QMALookAtPlugin::start()
{
    /* do nothing */
}

void QMALookAtPlugin::stop()
{
    /* do nothing */
}

void QMALookAtPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
    Q_UNUSED(command);
    Q_UNUSED(arguments);
    /* do nothing */
}

void QMALookAtPlugin::receiveEvent(const QString &type, const QStringList &arguments)
{
    if (type == MMDAI::SceneEventHandler::kKeyEvent && arguments.at(0) == "L") {
        int count = m_controller->countPMDObjects();
        for (int i = 0; i < count; i++) {
            MMDAI::PMDObject *object = m_controller->getPMDObject(i);
            if(object->isEnable()) {
                if(m_enable == true) {
                    m_neckController[i].setEnableFlag(false);
                    m_eyeController[i].setEnableFlag(false);
                }
                else {
                    m_neckController[i].setEnableFlag(true);
                    m_eyeController[i].setEnableFlag(true);
                }
            }
        }
        m_enable = !m_enable;
    }
    else if (type == MMDAI::SceneEventHandler::kModelChangeEvent || type == MMDAI::SceneEventHandler::kModelAddEvent) {
        QTextCodec *codec = QTextCodec::codecForName("UTF8");
        QString target = arguments.at(0);
        int count = m_controller->countPMDObjects();
        for (int i = 0; i < count; i++) {
            MMDAI::PMDObject *object = m_controller->getPMDObject(i);
            const char *a = object->getAlias();
            if (a) {
                QString alias = codec->toUnicode(a, strlen(a));
                if (alias == target) {
                    MMDAI::PMDModel *model = object->getPMDModel();
                    setNeckController(&m_neckController[i], model);
                    setEyeController(&m_eyeController[i], model);
                }
            }
        }
    }
}

void QMALookAtPlugin::update(const QRect &rect, const QPoint &pos, const double delta)
{
    QPoint p = pos;
    btVector3 pointPos;

    /* set target position */
    p.setX(pos.x() - ((rect.left() + rect.right()) / 2));
    p.setY(pos.y() - ((rect.top() + rect.bottom()) / 2));
    float rate = 100.0f / static_cast<float>(rect.right() - rect.left());
    pointPos.setValue(p.x() * rate, -p.y() * rate, 0.0f);
    btVector3 targetPos = m_controller->getScreenPointPosition(pointPos);

    /* calculate direction of all controlled bones */
    int count = m_controller->countPMDObjects();
    for (int i = 0; i < count; i++) {
        MMDAI::PMDObject *object = m_controller->getPMDObject(i);
        if (object->isEnable()) {
            m_neckController[i].update(&targetPos, static_cast<float>(delta));
            m_eyeController[i].update(&targetPos, static_cast<float>(delta));
        }
    }
}

void QMALookAtPlugin::prerender()
{
    /* do nothing */
}

void QMALookAtPlugin::postrender()
{
    /* do nothing */
}

Q_EXPORT_PLUGIN2(qma_lookat_plugin, QMALookAtPlugin);
