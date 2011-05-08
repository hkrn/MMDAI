/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include "QMAOpenJTalkPlugin.h"

#include <QtCore>
#include <MMDME/Common.h>

const QString kOpenJTalkStartCommand = "SYNTH_START";
const QString kOpenJTalkStopCommand =  "SYNTH_STOP";

const QString QMAOpenJTalkPlugin::kSynthEventStart = "SYNTH_EVENT_START";
const QString QMAOpenJTalkPlugin::kSynthEventStop = "SYNTH_EVENT_STOP";
const QString QMAOpenJTalkPlugin::kLipSyncStart = "LIPSYNC_START";
const QString QMAOpenJTalkPlugin::kLipSyncStop = "LIPSYNC_STOP";

const QByteArray kOpenJTalkCodecName = "Shift-JIS";

QMAOpenJTalkPlugin::QMAOpenJTalkPlugin(QObject *parent)
    : QMAPlugin(parent)
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
        MMDAILogWarn("Pa_Initialized failed: %s", Pa_GetErrorText(err));
}

QMAOpenJTalkPlugin::~QMAOpenJTalkPlugin()
{
    qDeleteAll(m_models);
    PaError err = Pa_Terminate();
    if (err != paNoError)
        MMDAILogWarn("Pa_Terminate failed: %s", Pa_GetErrorText(err));
}

void QMAOpenJTalkPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(controller);
    if (m_base.isEmpty()) {
        QDir dir("MMDAIUserData:/");
        m_base = dir.absolutePath();
        m_config = dir.absoluteFilePath(QString("%1.ojt").arg(baseName));
        m_dir = QDir("MMDAIResources:/").absoluteFilePath("AppData/Open_JTalk");
    }
}

void QMAOpenJTalkPlugin::unload()
{
}

void QMAOpenJTalkPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    if (command == kOpenJTalkStartCommand && arguments.count() == 3) {
        QString name = arguments[0].toString();
        QString style = arguments[1].toString();
        QString text = arguments[2].toString();
        QtConcurrent::run(this, &QMAOpenJTalkPlugin::run, name, style, text);
    }
    else if (command == kOpenJTalkStopCommand && arguments.count() == 1) {
        QString name = arguments[0].toString();
        Q_UNUSED(name);
    }
}

void QMAOpenJTalkPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    Q_UNUSED(type);
    Q_UNUSED(arguments);
    /* do nothing */
}

void QMAOpenJTalkPlugin::run(const QString &name, const QString &style, const QString &text)
{
    QMAOpenJTalkModel *model;
    if (m_models.contains(name)) {
        model = m_models[name];
    }
    else {
        model = new QMAOpenJTalkModel(this);
        model->loadSetting(m_base, m_config);
        model->loadDictionary(m_dir);
        m_models[name] = model;
    }
    model->setStyle(style);
    model->setText(text);
    QString sequence = model->getPhonemeSequence();
    QByteArray bytes = model->finalize(false);

    QList<QVariant> arguments;
    arguments << name;
    eventPost(kSynthEventStart, arguments);
    arguments.clear();
    arguments << name << sequence;
    commandPost(kLipSyncStart, arguments);

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
    err = Pa_OpenStream(&stream, NULL, &output, QMAOpenJTalkModel::kSamplingRate, 1024, paClipOff, NULL, NULL);
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
        err = Pa_WriteStream(stream, bytes.constData(), bytes.size() / sizeof(short));
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

final:
    arguments.clear();
    arguments << name;
    eventPost(kSynthEventStop, arguments);
}

Q_EXPORT_PLUGIN2(qma_openjtalk_plugin, QMAOpenJTalkPlugin)
