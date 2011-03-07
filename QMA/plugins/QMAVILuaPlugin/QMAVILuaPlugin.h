#ifndef QMAVILUAPLUGIN_H
#define QMAVILUAPLUGIN_H

#include <lua.hpp>
#include <QMap>

#include "QMAPlugin.h"
#include "CommandDispatcher.h"

class QMAVILuaPlugin : public QMAPlugin
{
  Q_OBJECT
  Q_INTERFACES(QMAPlugin)

public:
  QMAVILuaPlugin();
  ~QMAVILuaPlugin();

public slots:
  void initialize(MMDAI::SceneController *controller);
  void start();
  void stop();
  void receiveCommand(const QString &command, const QStringList &arguments);
  void receiveEvent(const QString &type, const QStringList &arguments);
  void update(const QRect &rect, const QPoint &pos, const double delta);
  void prerender();
  void postrender();

  void pushCommand(const QString &command, const QStringList &arguments);
  void pushEvent(const QString &command, const QStringList &arguments);
  void broadcast();

signals:
  void commandPost(const QString &command, const QStringList &arguments);
  void eventPost(const QString &type, const QStringList &arguments);

private:
  void handleInvoke(const QString &command, const QStringList &arguments, const char *invoke);

  QMap<QString, QStringList> m_commands;
  QMap<QString, QStringList> m_events;
  lua_State *m_state;
};

#endif // QMAVILUAPLUGIN_H
