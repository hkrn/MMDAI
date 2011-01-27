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

#ifndef QMAJULIUSPLUGIN_H
#define QMAJULIUSPLUGIN_H

#include "CommandDispatcher.h"
#include "QMAJuliusInitializer.h"
#include "Julius_Thread.h"

/*
  X11 defines "Status" as int, it causes an error at compile time.
  We must load qdatastream.h before X11 headers.
  see http://qt.nokia.com/developer/task-tracker/index_html?method=entry&id=96943
*/
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "QMAPlugin.h"

class QMAJuliusPlugin : public QMAPlugin, public CommandDispatcher
{
  Q_OBJECT
  Q_INTERFACES(QMAPlugin)

public:
  QMAJuliusPlugin(QObject *parent = 0);
  ~QMAJuliusPlugin();

  void sendCommand(const char *command, char *arguments);
  void sendEvent(const char *type, char *arguments);

public slots:
  void initialize(SceneController *controller, const QString &path);
  void start();
  void stop();
  void receiveCommand(const QString &command, const QStringList &arguments);
  void receiveEvent(const QString &type, const QStringList &arguments);
  void update(const QRect &rect, double delta);
  void render();

private slots:
  void startJuliusEngine();

signals:
  void commandPost(const QString &command, const QStringList &arguments);
  void eventPost(const QString &type, const QStringList &arguments);

private:
  QMAJuliusInitializer *m_initializer;
  Julius_Thread *m_thread;
};

#endif // QMAJULIUSPLUGIN_H
