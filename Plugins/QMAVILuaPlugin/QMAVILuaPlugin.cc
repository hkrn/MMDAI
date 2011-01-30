#include "QMAVILuaPlugin.h"

static int mmdai_command(lua_State *state)
{
  int top = lua_gettop(state);
  QMAVILuaPlugin *plugin = static_cast<QMAVILuaPlugin *>(lua_touserdata(state, lua_upvalueindex(1)));
  const char *command = lua_tostring(state, 1);
  if (plugin != NULL && command != NULL) {
    QStringList arguments;
    for (int i = 2; i <= top; i++) {
      const char *s = lua_tostring(state, i);
      arguments << QString(s);
    }
    plugin->pushCommand(QString(command), arguments);
  }
  lua_settop(state, top);
  return 1;
}

static int mmdai_event(lua_State *state)
{
  int top = lua_gettop(state);
  QMAVILuaPlugin *plugin = static_cast<QMAVILuaPlugin *>(lua_touserdata(state, lua_upvalueindex(1)));
  const char *type = lua_tostring(state, 1);
  if (plugin != NULL && type != NULL) {
    QStringList arguments;
    for (int i = 2; i <= top; i++) {
      const char *s = lua_tostring(state, i);
      arguments << QString(s);
    }
    plugin->pushEvent(QString(type), arguments);
  }
  lua_settop(state, top);
  return 1;
}

QMAVILuaPlugin::QMAVILuaPlugin()
  : m_state(0)
{
  m_state = lua_open();
  luaL_openlibs(m_state);
}

QMAVILuaPlugin::~QMAVILuaPlugin()
{
  lua_close(m_state);
  m_state = 0;
}

void QMAVILuaPlugin::initialize(SceneController *controller)
{
  static struct luaL_reg funcs[] = {
    { "command", mmdai_command },
    { "event", mmdai_event },
  };
  Q_UNUSED(controller);
  lua_newtable(m_state);
  for (unsigned int i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++) {
    lua_pushstring(m_state, funcs[i].name);
    lua_pushlightuserdata(m_state, this);
    lua_pushcclosure(m_state, funcs[i].func, 1);
    lua_settable(m_state, -3);
  }
  lua_setglobal(m_state, "mmdai");
}

void QMAVILuaPlugin::start()
{
  QString path = QFile("mmdai:/MMDAI.lua").fileName();
  int top = lua_gettop(m_state);
  int ret = luaL_dofile(m_state, path.toUtf8().constData());
  if (ret != 0) {
    qWarning("Executing a lua script error: %s", lua_tostring(m_state, -1));
    lua_pop(m_state, 1);
  }
  lua_settop(m_state, top);
  broadcast();
}

void QMAVILuaPlugin::stop()
{
  /* do nothing */
}

void QMAVILuaPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  handleInvoke(command, arguments, "mmdai_handle_command");
}

void QMAVILuaPlugin::receiveEvent(const QString &type, const QStringList &arguments)
{
  handleInvoke(type, arguments, "mmdai_handle_event");
}

void QMAVILuaPlugin::handleInvoke(const QString &command, const QStringList &arguments, const char *invoke)
{
  int top = lua_gettop(m_state);
  lua_getglobal(m_state, invoke);
  lua_pushstring(m_state, command.toUtf8().constData());
  foreach (QString argv, arguments) {
    lua_pushstring(m_state, argv.toUtf8().constData());
  }
  int ret = lua_pcall(m_state, arguments.count() + 1, 0, 0);
  if (ret != 0) {
    qWarning("Invoke %s error: %s", invoke, lua_tostring(m_state, -1));
    lua_pop(m_state, 1);
  }
  lua_settop(m_state, top);
  broadcast();
}

void QMAVILuaPlugin::update(const QRect &rect, const QPoint &pos, const double delta)
{
  Q_UNUSED(rect);
  Q_UNUSED(pos);
  Q_UNUSED(delta);
  /* do nothing */
}

void QMAVILuaPlugin::render()
{
  /* do nothing */
}

void QMAVILuaPlugin::pushCommand(const QString &command, const QStringList &arguments)
{
  m_commands.insert(command, arguments);
}

void QMAVILuaPlugin::pushEvent(const QString &command, const QStringList &arguments)
{
  m_events.insert(command, arguments);
}

void QMAVILuaPlugin::broadcast()
{
  QMap<QString, QStringList> commands = m_commands;
  m_commands.clear();
  QMapIterator<QString, QStringList> commandIterator(commands);
  while (commandIterator.hasNext()) {
    commandIterator.next();
    emit commandPost(commandIterator.key(), commandIterator.value());
  }
  QMap<QString, QStringList> events = m_events;
  m_events.clear();
  QMapIterator<QString, QStringList> eventIterator(events);
  while (eventIterator.hasNext()) {
    eventIterator.next();
    emit eventPost(eventIterator.key(), eventIterator.value());
  }
}

Q_EXPORT_PLUGIN2("QMAVILuaPlugin", QMAVILuaPlugin);
