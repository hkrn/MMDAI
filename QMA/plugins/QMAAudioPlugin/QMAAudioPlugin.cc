/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include "QMAAudioPlugin.h"

#include <QtCore>

const QString QMAAudioPlugin::kSoundStart = "SOUND_START";
const QString QMAAudioPlugin::kSoundStop = "SOUND_STOP";
const QString QMAAudioPlugin::kSoundStartEvent = "SOUND_EVENT_START";
const QString QMAAudioPlugin::kSoundStopEvent = "SOUND_EVENT_STOP";

QMAAudioPlugin::QMAAudioPlugin(QObject *parent)
    : QMAPlugin(parent),
      m_audioOutput(new Phonon::AudioOutput(Phonon::MusicCategory, this)),
      m_audioObject(new Phonon::MediaObject(this))
{
    Phonon::createPath(m_audioObject, m_audioOutput);
    connect(m_audioObject, SIGNAL(aboutToFinish()),
            this, SLOT(aboutToFinish()));
    connect(m_audioObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(changeCurrentSource(Phonon::MediaSource)));
    connect(m_audioObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(changeState(Phonon::State,Phonon::State)));
}

QMAAudioPlugin::~QMAAudioPlugin()
{
    delete m_audioOutput;
    delete m_audioObject;
}

void QMAAudioPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(controller);
    Q_UNUSED(baseName);
}

void QMAAudioPlugin::unload()
{
    m_audioObject->clear();
    m_audioSources.clear();
}

void QMAAudioPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    int argc = arguments.count();
    if (command == kSoundStart && argc >= 2) {
        QString alias = arguments.at(0).toString();
        QString filename = arguments.at(1).toString();
        if (!QDir::isAbsolutePath(filename))
            filename = QDir("MMDAIUserData:/").absoluteFilePath(filename);
        Phonon::MediaSource source(filename);
        m_audioSources[alias] = source;
        m_audioObject->enqueue(source);
        m_audioObject->play();
        emit eventPost(kSoundStartEvent, arguments);
    }
    else if (command == kSoundStop && argc >= 1) {
        QString alias = arguments.at(0).toString();
        if (m_audioSources.contains(alias)) {
            m_audioObject->stop();
            m_audioSources.remove(alias);
            m_audioObject->setQueue(m_audioSources.values());
        }
    }
}

void QMAAudioPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    Q_UNUSED(type);
    Q_UNUSED(arguments);
    /* do nothing */
}

void QMAAudioPlugin::aboutToFinish()
{
    QList<QVariant> arguments;
    QString key = m_audioSources.key(m_audioObject->currentSource());
    m_audioSources.remove(key);
    arguments << key;
    emit eventPost(kSoundStopEvent, arguments);
}

void QMAAudioPlugin::changeCurrentSource(Phonon::MediaSource source)
{
    QList<QVariant> arguments;
    QString key = m_audioSources.key(source);
    m_audioSources.remove(key);
    arguments << key << source.fileName();
    emit eventPost(kSoundStartEvent, arguments);
}

void QMAAudioPlugin::changeState(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);
    if (newState == Phonon::ErrorState) {
        qWarning() << "QMAAudioPlugin catched an error:" << m_audioObject->errorString();
    }
}

Q_EXPORT_PLUGIN2(qma_audio_plugin, QMAAudioPlugin)
