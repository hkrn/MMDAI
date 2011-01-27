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

#include "QMAJuliusPlugin.h"

QMAJuliusPlugin::QMAJuliusPlugin(QObject *parent)
  : QMAPlugin(parent),
  m_initializer(NULL),
  m_thread(NULL)
{
}

QMAJuliusPlugin::~QMAJuliusPlugin()
{
  delete m_initializer;
  delete m_thread;
}

void QMAJuliusPlugin::initialize(SceneController *controller, const QString &path)
{
  Q_UNUSED(controller);
  QDir dir = QDir(QString(path) + "/AppData/Julius");
  QString filename = dir.absoluteFilePath("jconf.txt");
  QFile jconf(filename);
  QStringList conf;
  if (jconf.open(QFile::ReadOnly)) {
    QTextStream stream(&jconf);
    while (!stream.atEnd()) {
      QString line = stream.readLine();
      QStringList pair = line.split(QRegExp("\\s+"));
      if (pair.size() == 2) {
        QString key = pair[0];
        QString value = pair[1];
        if (key == "-d" || key == "-v" || key == "-h" || key == "-hlist") {
          QDir path(dir);
          value = path.absoluteFilePath(value);
        }
        conf << key << value;
      }
      else {
        foreach (QString value, pair) {
          conf << value;
        }
      }
    }
    m_initializer = new QMAJuliusInitializer(conf);
    connect(m_initializer, SIGNAL(finished()), this, SLOT(startJuliusEngine()));
    m_initializer->start();
  }
}

void QMAJuliusPlugin::start()
{
  /* do nothing */
}

void QMAJuliusPlugin::stop()
{
  /* do nothing */
}

void QMAJuliusPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  Q_UNUSED(command);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAJuliusPlugin::receiveEvent(const QString &type, const QStringList &arguments)
{
  Q_UNUSED(type);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAJuliusPlugin::update(const QRect &rect, const QPoint &pos, const double delta)
{
  Q_UNUSED(rect);
  Q_UNUSED(pos);
  Q_UNUSED(delta);
  /* do nothing */
}

void QMAJuliusPlugin::render()
{
  /* do nothing */
}

void QMAJuliusPlugin::startJuliusEngine()
{
  m_thread = new Julius_Thread(this, m_initializer);
  m_thread->start();
}

void QMAJuliusPlugin::sendCommand(const char *command, char *arguments)
{
  emit commandPost(QString(command), QString(arguments).split('|'));
  if (arguments != NULL)
    free(arguments);
}

void QMAJuliusPlugin::sendEvent(const char */*type*/, char */*arguments*/)
{
  /* do nothing */
}

Q_EXPORT_PLUGIN2("QMAJuliusPlugin", QMAJuliusPlugin)
