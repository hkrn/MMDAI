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

#include "QMAOpenJTalkPlugin.h"

#include <QDir>
#include <QTextCodec>
#include <stdlib.h>

#define PLUGINOPENJTALK_STARTCOMMAND "SYNTH_START"
#define PLUGINOPENJTALK_STOPCOMMAND  "SYNTH_STOP"

QMAOpenJTalkPlugin::QMAOpenJTalkPlugin(QObject *parent)
  : QMAPlugin(parent),
    m_thread(new Open_JTalk_Thread(this))
{
}

QMAOpenJTalkPlugin::~QMAOpenJTalkPlugin()
{
  delete m_thread;
}

void QMAOpenJTalkPlugin::initialize(SceneController *controller)
{
  Q_UNUSED(controller);
  QString dir = QDir("mmdai:AppData/Open_JTalk").absolutePath();
  QString config = QFile("mmdai:MMDAI.ojt").fileName();
  m_thread->load(dir.toUtf8().constData(), config.toUtf8().constData());
  m_thread->start();
}

void QMAOpenJTalkPlugin::start()
{
  /* do nothing */
}

void QMAOpenJTalkPlugin::stop()
{
  /* do nothing */
}

void QMAOpenJTalkPlugin::createWindow()
{
  /* do nothing */
}

void QMAOpenJTalkPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  if (command == PLUGINOPENJTALK_STARTCOMMAND) {
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QString str = arguments.join("|");
    const char *argv = codec->fromUnicode(str).constData();
    m_thread->setSynthParameter(argv);
  }
  else if (command == PLUGINOPENJTALK_STOPCOMMAND) {
    m_thread->stop();
  }
}

void QMAOpenJTalkPlugin::receiveEvent(const QString &type, const QStringList &arguments)
{
  Q_UNUSED(type);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAOpenJTalkPlugin::update(const QRect &rect, const QPoint &pos, const double delta)
{
  Q_UNUSED(rect);
  Q_UNUSED(pos);
  Q_UNUSED(delta);
  /* do nothing */
}

void QMAOpenJTalkPlugin::render()
{
  /* do nothing */
}

void QMAOpenJTalkPlugin::sendCommand(const char *command, char *arguments)
{
  emit commandPost(QString(command), QString(arguments).split('|'));
  free(arguments);
}

void QMAOpenJTalkPlugin::sendEvent(const char *type, char *arguments)
{
  emit eventPost(QString(type), QString(arguments).split('|'));
  free(arguments);
}

Q_EXPORT_PLUGIN2("QMAOpenJTalkPlugin", QMAOpenJTalkPlugin)
