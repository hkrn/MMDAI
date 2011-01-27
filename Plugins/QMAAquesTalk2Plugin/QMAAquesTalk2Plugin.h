#ifndef QMAAQUESTALK2PLUGIN_H
#define QMAAQUESTALK2PLUGIN_H

#include <phonon/AudioOutput>
#include <phonon/MediaObject>

#include <QBuffer>
#include <QByteArray>

#include "QMAPlugin.h"

class QMAAquesTalk2Plugin : public QMAPlugin
{
  Q_OBJECT
  Q_INTERFACES(QMAPlugin)

public:
  QMAAquesTalk2Plugin(QObject *parent = 0);
  ~QMAAquesTalk2Plugin();

public slots:
  void initialize(SceneController *controller, const QString &path);
  void start();
  void stop();
  void receiveCommand(const QString &command, const QStringList &arguments);
  void receiveEvent(const QString &type, const QStringList &arguments);
  void update(const QRect &rect, double delta);
  void render();

  void finished();
  void stateChanged(Phonon::State newState, Phonon::State oldState);

signals:
  void commandPost(const QString &command, const QStringList &arguments);
  void eventPost(const QString &type, const QStringList &arguments);

private:
  Phonon::AudioOutput *m_output;
  Phonon::MediaObject *m_object;
  QBuffer *m_buffer;
};

#endif // QMAAQUESTALK2PLUGIN_H
