#ifndef QMAAUDIOPLUGIN_H
#define QMAAUDIOPLUGIN_H

#include <QDir>
#include <QMap>
#include <phonon/AudioOutput>
#include <phonon/MediaObject>

#include "QMAPlugin.h"

class QMAAudioPlugin : public QMAPlugin
{
  Q_OBJECT
  Q_INTERFACES(QMAPlugin)

public:
  QMAAudioPlugin(QObject *parent = 0);
  ~QMAAudioPlugin();

public slots:
  void initialize(const QString &path);
  void start();
  void stop();
  void createWindow();
  void receiveCommand(const QString &command, const QStringList &arguments);
  void receiveEvent(const QString &type, const QStringList &arguments);
  void update(const QRect &rect, const double delta);
  void render();

private slots:
  void aboutToFinish();
  void changeCurrentSource(Phonon::MediaSource source);
  void changeState(Phonon::State newState, Phonon::State oldState);

signals:
  void commandPost(const QString &command, const QStringList &arguments);
  void eventPost(const QString &type, const QStringList &arguments);

private:
  Phonon::AudioOutput *m_audioOutput;
  Phonon::MediaObject *m_audioObject;
  QDir m_path;
  QMap<QString, Phonon::MediaSource> m_audioSources;
};

#endif // QMAAUDIOPLUGIN_H
