#include "QMAAquesTalk2Plugin.h"

#ifdef Q_OS_DARWIN
#include <AquesTalk2/AquesTalk2.h>
#else
#include "AquesTalk2.h"
#endif

QMAAquesTalk2Plugin::QMAAquesTalk2Plugin(QObject *parent) :
    QMAPlugin(parent)
{
  m_object = new Phonon::MediaObject(this);
  m_output = new Phonon::AudioOutput(Phonon::MusicCategory, this);
  Phonon::createPath(m_object, m_output);
}

QMAAquesTalk2Plugin::~QMAAquesTalk2Plugin()
{
  delete m_output;
  delete m_object;
}

void QMAAquesTalk2Plugin::initialize(SceneController */*controller*/, const QString &/*path*/)
{
}

void QMAAquesTalk2Plugin::start(SceneController */*controller*/)
{
}

void QMAAquesTalk2Plugin::stop(SceneController */*controller*/)
{
}

void QMAAquesTalk2Plugin::createWindow(SceneController */*controller*/)
{
}

void QMAAquesTalk2Plugin::receiveCommand(SceneController */*controller*/, const QString &/*command*/, const QString &/*arguments*/)
{
}

void QMAAquesTalk2Plugin::receiveEvent(SceneController */*controller*/, const QString &/*type*/, const QString &/*arguments*/)
{
}

void QMAAquesTalk2Plugin::update(SceneController */*controller*/, const QRect &/*rect*/, double /*delta*/)
{
}

void QMAAquesTalk2Plugin::render(SceneController */*controller*/)
{
}
