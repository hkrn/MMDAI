#include "QMAAudioPlugin.h"

#include <QFile>

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

void QMAAudioPlugin::initialize(const QString &path)
{
  m_path = QDir(path);
}

void QMAAudioPlugin::start()
{
  /* do nothing */
}

void QMAAudioPlugin::stop()
{
  m_audioObject->clear();
  m_audioSources.clear();
}

void QMAAudioPlugin::createWindow()
{
  /* do nothing */
}

void QMAAudioPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  int argc = arguments.count();
  if (command == "SOUND_START" && argc == 2) {
    QString alias = arguments.at(0);
    QString filename = arguments.at(1);
    if (!QDir::isAbsolutePath(filename))
      filename = m_path.absoluteFilePath(arguments.at(1));
    Phonon::MediaSource source(filename);
    m_audioSources[alias] = source;
    m_audioObject->enqueue(source);
    m_audioObject->play();
    emit eventPost(QString("SOUND_EVENT_START"), arguments);
  }
  else if (command == "SOUND_STOP" && argc == 1) {
    QString alias = arguments.at(0);
    if (m_audioSources.contains(alias)) {
      m_audioObject->stop();
      m_audioSources.remove(alias);
      m_audioObject->setQueue(m_audioSources.values());
    }
  }
}

void QMAAudioPlugin::receiveEvent(const QString &/*type*/, const QStringList &/*arguments*/)
{
  /* do nothing */
}

void QMAAudioPlugin::update(const QRect &/*rect*/, const double /*delta*/)
{
  /* do nothing */
}

void QMAAudioPlugin::render()
{
  /* do nothing */
}

void QMAAudioPlugin::aboutToFinish()
{
  QStringList arguments;
  QString key = m_audioSources.key(m_audioObject->currentSource());
  m_audioSources.remove(key);
  arguments << key;
  emit eventPost("SOUND_EVENT_STOP", arguments);
}

void QMAAudioPlugin::changeCurrentSource(Phonon::MediaSource source)
{
  QStringList arguments;
  QString key = m_audioSources.key(source);
  m_audioSources.remove(key);
  arguments << key << source.fileName();
  emit eventPost("SOUND_EVENT_START", arguments);
}

void QMAAudioPlugin::changeState(Phonon::State newState, Phonon::State oldState)
{
  Q_UNUSED(oldState);
  if (newState == Phonon::ErrorState) {
    qWarning() << "QMAAudioPlugin catched an error:" << m_audioObject->errorString();
  }
}

Q_EXPORT_PLUGIN2("QMAAudioPlugin", QMAAudioPlugin)
