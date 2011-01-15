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

#include "QMABoneControlPlugin.h"

#include <QEvent>

QMABoneControlPlugin::QMABoneControlPlugin(QObject *parent)
  : QMAPlugin(parent),
    m_num(2)
{
  BoneControlDef *def;
  const char defNameNeck[] = { 0x90, 0x55, 0x8c, 0xfc, 0x8e, 0xf0 }; /* 振向首 in Shift_JIS */
  const char boneNameHead[] = { 0x93, 0xaa }; /* 頭 in Shift_JIS */
  const char defNameEye[] = { 0x90, 0x55, 0x8c, 0xfc, 0x96, 0xda }; /* 振向目 in Shift_JIS */
  const char boneNameRightEye[] = { 0x89, 0x45, 0x96, 0xda }; /* 右目 in Shift_JIS */
  const char boneNameLeftEye[] = { 0x8d, 0xb6, 0x96, 0xda }; /* 左目 in Shift_JIS  */

  /* neck */
  def = &m_controllerDef[0];
  strncpy(def->name, defNameNeck, 20);
  def->boneNum = 1;
  strncpy(def->boneName[0], boneNameHead, 20);
  def->baseVector = btVector3(0.0, 0.0, 1.0);
  def->rateOn = 0.150f;
  def->rateOff = 0.008f;
  def->LowerAngularLimit = btVector3(MMDFILES_RAD(-45.0f), MMDFILES_RAD(-60.0f), MMDFILES_RAD(0.0f));
  def->UpperAngularLimit = btVector3(MMDFILES_RAD(20.0f), MMDFILES_RAD(60.0f), MMDFILES_RAD(0.0f));
  def->adjustPosition = btVector3(0.0f, -1.0f, 0.0f);

  /* eye */
  def = &m_controllerDef[1];
  strncpy(def->name, defNameEye, 20);
  def->boneNum = 2;
  strncpy(def->boneName[0], boneNameRightEye, 20);
  strncpy(def->boneName[1], boneNameLeftEye, 20);
  def->baseVector = btVector3(0.0, 0.0, 1.0);
  def->rateOn = 0.180f;
  def->rateOff = 0.008f;
  def->LowerAngularLimit = btVector3(MMDFILES_RAD(-5.0f), MMDFILES_RAD(-5.0f), MMDFILES_RAD(0.0f));
  def->UpperAngularLimit = btVector3(MMDFILES_RAD(5.0f), MMDFILES_RAD(5.0f), MMDFILES_RAD(0.0f));
  def->adjustPosition = btVector3(0.0f, 0.0f, 0.0f);
}

QMABoneControlPlugin::~QMABoneControlPlugin()
{
}

void QMABoneControlPlugin::initialize(SceneController * /* controller */, const QString & /* path */)
{
  /* do nothing */
}

void QMABoneControlPlugin::start(SceneController *controller)
{
  int length = controller->countPMDObjects();
  for (int i = 0; i < length; i++) {
    PMDObject *object = controller->getPMDObject(i);
    if (object->isEnable()) {
      PMDModel *model = object->getPMDModel();
      BoneController *boneController = m_boneController[i];
      for (int j = 0; j < m_num; j++) {
        boneController[j].initialize(model, &m_controllerDef[j]);
      }
    }
  }
}

void QMABoneControlPlugin::stop(SceneController * /*controller */)
{
  /* do nothing */
}

void QMABoneControlPlugin::createWindow(SceneController * /* controller */)
{
  /* do nothing */
}

void QMABoneControlPlugin::receiveCommand(SceneController */*controller*/, const QString &/*command*/, const QString &/*arguments*/)
{
  /* do nothing */
}

void QMABoneControlPlugin::receiveEvent(SceneController *controller, const QString &type, const QString &arguments)
{
  if (type == MMDAGENT_EVENT_MODEL_CHANGE) {
    char *ptr = NULL, *buf = strdup(arguments.toUtf8().constData());
    if (buf != NULL) {
      char *p = strtok_r(buf, "|", &ptr);
      int i = 0;
      for (; p != NULL; strtok_r(buf, "L", &ptr), i++) {
        if (i == 0) {
          PMDObject *object = controller->findPMDObjectByAlias(p);
          if (object != NULL) {
            BoneController *boneController = m_boneController[i];
            for (int j = 0; j < m_num; j++) {
              boneController[j].initialize(object->getPMDModel(), &m_controllerDef[j]);
            }
          }
        }
      }
      free(buf);
    }
  }
}

void QMABoneControlPlugin::update(SceneController *controller, const QRect &rect, const double delta)
{
  btVector3 src, dst;
  float rate = 100.0f / (float)(rect.right() - rect.left());
  float x = (rect.x() - (rect.left() + rect.right()) / 2) * rate;
  float y = (rect.y() - (rect.top() + rect.bottom()) / 2) * rate;
  src.setValue(x, y, 0.0f);
  controller->getScreenPointPosition(&dst, &src);
  int length = controller->countPMDObjects();
  for (int i = 0; i < length; i++) {
    PMDObject *object = controller->getPMDObject(i);
    BoneController *boneController = m_boneController[i];
    if (object->isEnable()) {
      for (int j = 0; j < m_num; j++) {
        boneController[j].update(&dst, (float)delta);
      }
    }
  }
}

void QMABoneControlPlugin::render(SceneController * /* controller */)
{
  /* do nothing */
}

Q_EXPORT_PLUGIN2("QMABoneControlPlugin", QMABoneControlPlugin)
