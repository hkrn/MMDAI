/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
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

#include "QMAAquesTalk2Plugin.h"

#ifdef Q_OS_DARWIN
#include <AquesTalk2/AquesTalk2.h>
#else
#include "AquesTalk2.h"
#endif

QMAAquesTalk2Plugin::QMAAquesTalk2Plugin(QObject *parent)
  : QMAPlugin(parent),
    m_output(new Phonon::AudioOutput(Phonon::GameCategory, this)),
    m_object(new Phonon::MediaObject(this)),
    m_buffer(0)
{
  Phonon::createPath(m_object, m_output);
  connect(m_object, SIGNAL(finished()), this, SLOT(finished()));
  connect(m_object, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
          this, SLOT(stateChanged(Phonon::State,Phonon::State)));
}

QMAAquesTalk2Plugin::~QMAAquesTalk2Plugin()
{
  delete m_buffer;
  delete m_output;
  delete m_object;
}

void QMAAquesTalk2Plugin::initialize(SceneController */*controller*/, const QString &path)
{
  Q_UNUSED(path);
  /* do nothing */
}

void QMAAquesTalk2Plugin::start()
{
  /* do nothing */
}

void QMAAquesTalk2Plugin::stop()
{
  m_object->stop();
}

void QMAAquesTalk2Plugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  int argc = arguments.count();
  if (command == "MMDAI_AQTK2_START" && argc > 0) {
    int size = 0;
    const char *text = arguments.at(0).toAscii().constData();
    unsigned char *data = AquesTalk2_Synthe_Utf8(text, 100, &size, NULL);
    if (data != NULL) {
      delete m_buffer;
      m_buffer = new QBuffer();
      m_buffer->open(QBuffer::ReadWrite);
      m_buffer->write((const char *)data, size);
      m_object->setCurrentSource(m_buffer);
      m_object->play();
    }
    AquesTalk2_FreeWave(data);
  }
}

void QMAAquesTalk2Plugin::receiveEvent(const QString &type, const QStringList &arguments)
{
  Q_UNUSED(type);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAAquesTalk2Plugin::update(const QRect &rect, double delta)
{
  Q_UNUSED(rect);
  Q_UNUSED(delta);
  /* do nothing */
}

void QMAAquesTalk2Plugin::render()
{
  /* do nothing */
}

void QMAAquesTalk2Plugin::finished()
{
  QStringList arguments;
  emit eventPost(QString("MMDAI_AQTK2_STOP"), arguments);
}

void QMAAquesTalk2Plugin::stateChanged(Phonon::State newState, Phonon::State oldState)
{
  Q_UNUSED(oldState);
  if (newState == Phonon::ErrorState)
    qWarning("Phonon error: %s", m_object->errorString().toAscii().constData());
}

Q_EXPORT_PLUGIN2("QMAAquesTalk2Plugin", QMAAquesTalk2Plugin)
