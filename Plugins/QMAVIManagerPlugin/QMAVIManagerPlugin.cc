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

#include "QMAVIManagerPlugin.h"

#include <QFile>
#include <QString>

QMAVIManagerPlugin::QMAVIManagerPlugin(QObject *parent)
  : QMAPlugin(parent),
    m_thread(this)
{
}

QMAVIManagerPlugin::~QMAVIManagerPlugin()
{
}

void QMAVIManagerPlugin::initialize(SceneController * /* controller */, const QString &path)
{
  QFile config(path + "/MMDAgent.fst");
  if (config.exists())
    m_thread.load(config.fileName().toUtf8().constData());
}

void QMAVIManagerPlugin::start(SceneController * /* controller */)
{
  m_thread.start();
}

void QMAVIManagerPlugin::stop(SceneController * /* controller */)
{
  /* do nothing */
}

void QMAVIManagerPlugin::createWindow(SceneController * /* controller */)
{
  /* do nothing */
}

void QMAVIManagerPlugin::receiveCommand(SceneController */*controller*/, const QString &/*command*/, const QString &/*arguments*/)
{
  /* do nothing */
}

void QMAVIManagerPlugin::receiveEvent(SceneController */*controller*/, const QString &type, const QString &arguments)
{
  if (m_thread.isStarted()) {
    m_thread.enqueueBuffer(type.toUtf8().constData(), arguments.toUtf8().constData());
  }
}

void QMAVIManagerPlugin::update(SceneController * /* controller */, const QRect & /* rect */, const double /* delta */)
{
  /* do nothing */
}

void QMAVIManagerPlugin::render(SceneController * /* controller */)
{
  /* do nothing */
}

void QMAVIManagerPlugin::sendCommand(const char *type, const char *arguments)
{
  emit commandPost(QString(type), QString(arguments));
}

Q_EXPORT_PLUGIN2("QMAVIManagerPlugin", QMAVIManagerPlugin)
