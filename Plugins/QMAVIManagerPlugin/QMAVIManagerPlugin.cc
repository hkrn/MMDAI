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

#include "QMAVIManagerPlugin.h"

#include <QFile>
#include <QTextCodec>
#include <stdlib.h>

QMAVIManagerPlugin::QMAVIManagerPlugin(QObject *parent)
  : QMAPlugin(parent),
    m_thread(this)
{
}

QMAVIManagerPlugin::~QMAVIManagerPlugin()
{
}

void QMAVIManagerPlugin::initialize(SceneController *controller)
{
  Q_UNUSED(controller);
  QFile config("mmdai:/MMDAI.fst");
  if (config.exists())
    m_thread.load(config.fileName().toUtf8().constData());
}

void QMAVIManagerPlugin::start()
{
  m_thread.start();
}

void QMAVIManagerPlugin::stop()
{
  m_thread.stop();
}

void QMAVIManagerPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  Q_UNUSED(command);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAVIManagerPlugin::receiveEvent(const QString &type, const QStringList &arguments)
{
  if (m_thread.isStarted()) {
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    m_thread.enqueueBuffer(type.toUtf8().constData(), codec->fromUnicode(arguments.join("|")).constData());
  }
}

void QMAVIManagerPlugin::update(const QRect &rect, const QPoint &pos, const double delta)
{
  Q_UNUSED(rect);
  Q_UNUSED(pos);
  Q_UNUSED(delta);
  /* do nothing */
}

void QMAVIManagerPlugin::render()
{
  /* do nothing */
}

void QMAVIManagerPlugin::sendCommand(const char *command, char *arguments)
{
  QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
  QString argv = codec->toUnicode(arguments, strlen(arguments));
  emit commandPost(QString(command), argv.split("|"));
  free(arguments);
}

void QMAVIManagerPlugin::sendEvent(const char *type, char *arguments)
{
  Q_UNUSED(type);
  Q_UNUSED(arguments);
  /* do nothing */
}

Q_EXPORT_PLUGIN2(qma_vimanager_plugin, QMAVIManagerPlugin)
