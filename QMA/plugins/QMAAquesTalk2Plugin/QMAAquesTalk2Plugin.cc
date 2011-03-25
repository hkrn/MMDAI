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

#include <QtConcurrentRun>
#include <QDir>
#include <QFile>
#include <QTextCodec>

#include <MMDAI/MMDAI.h>
#include "portaudio.h"

#ifdef Q_OS_DARWIN
#include <AquesTalk2/AquesTalk2.h>
#else
#include "AquesTalk2.h"
#endif

QMAAquesTalk2Plugin::QMAAquesTalk2Plugin(QObject *parent)
  : QMAPlugin(parent)
{
}

QMAAquesTalk2Plugin::~QMAAquesTalk2Plugin()
{
}

void QMAAquesTalk2Plugin::initialize(MMDAI::SceneController *controller)
{
  Q_UNUSED(controller);
  /* do nothing */
}

void QMAAquesTalk2Plugin::start()
{
  PaError err = Pa_Initialize();
  if (err != paNoError)
    MMDAILogWarn("Pa_Initialized failed: %s", Pa_GetErrorText(err));
}

void QMAAquesTalk2Plugin::stop()
{
  PaError err = Pa_Terminate();
  if (err != paNoError)
    MMDAILogWarn("Pa_Terminate failed: %s", Pa_GetErrorText(err));
}

void QMAAquesTalk2Plugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  int argc = arguments.count();
  if (command == "MMDAI_AQTK2_START" && argc >= 3) {
    QString text = arguments.at(2);
    QString phontPath = arguments.at(1);
    QString modelName = arguments.at(0);
    phontPath = QDir::isAbsolutePath(phontPath) ? phontPath : ("mmdai:" + phontPath);
    QtConcurrent::run(this, &QMAAquesTalk2Plugin::run, modelName, phontPath, text);
  }
}

void QMAAquesTalk2Plugin::receiveEvent(const QString &type, const QStringList &arguments)
{
  Q_UNUSED(type);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAAquesTalk2Plugin::update(const QRect &rect, const QPoint &pos, const double delta)
{
  Q_UNUSED(rect);
  Q_UNUSED(pos);
  Q_UNUSED(delta);
  /* do nothing */
}

void QMAAquesTalk2Plugin::prerender()
{
  /* do nothing */
}

void QMAAquesTalk2Plugin::postrender()
{
  /* do nothing */
}

void QMAAquesTalk2Plugin::run(const QString &modelName, const QString &phontPath, const QString &text)
{
  QFile phontFile(phontPath);
  QByteArray phont;
  char *ptr = 0;
  if (phontFile.exists() && phontFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
    phont = phontFile.readAll();
    ptr = phont.data();
  }

#ifdef Q_OS_WIN32
  QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
  unsigned char *data = AquesTalk2_Synthe(codec->fromUnicode(text).constData(), 100, &size, ptr);
#else
  int size = 0;
  unsigned char *data = AquesTalk2_Synthe_Utf8(text.toUtf8().constData(), 100, &size, ptr);
#endif
  if (data != NULL) {
    PaError err;
    PaStream *stream;
    PaStreamParameters output;
    output.device = Pa_GetDefaultOutputDevice();
    if (output.device == paNoDevice) {
      MMDAILogWarnString("No device to output found");
      goto final;
    }
    output.channelCount = 1;
    output.sampleFormat = paInt16;
    output.suggestedLatency = Pa_GetDeviceInfo(output.device)->defaultLowOutputLatency;
    output.hostApiSpecificStreamInfo = NULL;
    err = Pa_OpenStream(&stream, NULL, &output, 8000, 1024, paClipOff, NULL, NULL);
    if (err != paNoError) {
      MMDAILogWarn("Pa_OpenStream failed: %s", Pa_GetErrorText(err));
      goto final;
    }
    if (stream != NULL) {
      err = Pa_StartStream(stream);
      if (err != paNoError) {
        MMDAILogWarn("Pa_StartStream failed: %s", Pa_GetErrorText(err));
        goto final;
      }
      err = Pa_WriteStream(stream, data + 44, (size - 44) / sizeof(short));
      if (err != paNoError) {
        MMDAILogWarn("Pa_WriteStream failed: %s", Pa_GetErrorText(err));
        goto final;
      }
      err = Pa_CloseStream(stream);
      if (err != paNoError) {
        MMDAILogWarn("Pa_StartStream failed: %s", Pa_GetErrorText(err));
        goto final;
      }
    }

  }

final:
  AquesTalk2_FreeWave(data);

  QStringList arguments;
  arguments << modelName;
  emit eventPost(QString("MMDAI_AQTK2_STOP"), arguments);
}

Q_EXPORT_PLUGIN2(qma_aquestalk2_plugin, QMAAquesTalk2Plugin)
