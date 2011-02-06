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

const QString kOpenJTalkStartCommand = "SYNTH_START";
const QString kOpenJTalkStopCommand =  "SYNTH_STOP";
const QByteArray kOpenJTalkCodecName = "Shift-JIS";

QMAOpenJTalkPlugin::QMAOpenJTalkPlugin(QObject *parent)
  : QMAPlugin(parent),
    m_manager(new Open_JTalk_Manager(this))
{
}

QMAOpenJTalkPlugin::~QMAOpenJTalkPlugin()
{
  delete m_manager;
  m_manager = 0;
}

void QMAOpenJTalkPlugin::initialize(SceneController *controller)
{
  Q_UNUSED(controller);
  QString base = QDir::searchPaths("mmdai").at(0);
  QString dir = base + "/AppData/Open_JTalk";
  QString config = QFile("mmdai:MMDAI.ojt").fileName();
  m_manager->load(base.toUtf8().constData(), dir.toUtf8().constData(), config.toUtf8().constData());
  m_manager->start();
}

void QMAOpenJTalkPlugin::start()
{
  /* do nothing */
}

void QMAOpenJTalkPlugin::stop()
{
  /* do nothing */
}

void QMAOpenJTalkPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  QTextCodec *codec = QTextCodec::codecForName(kOpenJTalkCodecName);
  QString str = arguments.join("|");
  if (command == kOpenJTalkStartCommand) {
    m_manager->synthesis(codec->fromUnicode(str).constData());
  }
  else if (command == kOpenJTalkStopCommand) {
    m_manager->stop(codec->fromUnicode(str).constData());
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
  QTextCodec *codec = QTextCodec::codecForName(kOpenJTalkCodecName);
  emit commandPost(QString(command), codec->toUnicode(arguments).split('|'));
  free(arguments);
}

void QMAOpenJTalkPlugin::sendEvent(const char *type, char *arguments)
{
  QTextCodec *codec = QTextCodec::codecForName(kOpenJTalkCodecName);
  emit eventPost(QString(type), codec->toUnicode(arguments).split('|'));
  free(arguments);
}

Q_EXPORT_PLUGIN2("QMAOpenJTalkPlugin", QMAOpenJTalkPlugin)
